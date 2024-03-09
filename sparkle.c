/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2023 Beaver GEGL sparkle
 */

/*
Rough GEGL Graph of GEGL Sparkle so users can test it without installing. 


src aux=[ 

color value=#ff7aff
id=sparkle gimp:layer-mode layer-mode=divide opacity=0.05  aux=[  ref=sparkle cell-noise shape=1 scale=0.50  ]   
id=1 gimp:layer-mode layer-mode=color-erase  aux=[  ref=1 color value=#ff7aff  ]

 ]
crop

 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_color (color, _("Color"), "#ffffff")
    description (_("The main sparkle's color"))

property_color  (colorshadow, _("Shadow Color"), "#74e3ff")
  description   (_("The main sparkle's optional shadow color"))

property_double (x, _("Shadow Clone X"), -126)
  description   (_("Horizontal shadow offset"))
  ui_range      (-126.0, 126.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "x")

property_double (y, _("Shadow Clone  Y"), -8)
  description   (_("Vertical shadow offset"))
  ui_range      (-126.0, 126.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "y")

property_double (opacity, _("Shadow clone mode opacity --SLIDE TO ENABLE SHADOW"), 0.0)
  description   (_("A dropshadow that creates the apperance of a second sparkle field. It depends on the sparkle above as it is its drop shadow."))
  value_range   (0.0, 1.5)
  ui_steps      (0.1, 0.10)


property_double  (scale, _("Scale "), 0.24)
    description  (_("The scale of the noise function. (best at 0.10-0.40s) Lower is larger The internal cell noise powering the sparkle."))
  ui_range      (0.1, 0.7)
    value_range  (0, 1)

property_double  (shape, _("Shape"), 1.0)
    description  (_("Shape transition of the sparkle From a Diamond to a Circle"))
    value_range  (1.0, 2.0)

property_seed    (seed, _("Random seed"), rand)
    description  (_("The random seed for the noise function"))

property_double (opacitymeter, _("Hyper Opacity"), 1.3)
    description  (_("Opacity above 100% percent. This will make the sparkles bigger"))
  value_range   (0, 3.0)
  ui_range      (0.2, 3)
  ui_steps      (1.0, 3.0)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     sparkle
#define GEGL_OP_C_SOURCE sparkle.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *output, *color, *divide, *colorerase, *content, *crop, *opacity, *color2, *color3, *cellnoise, *ds;
  GeglColor *sparkle_hidden_color = gegl_color_new ("#ff7aff");
  GeglColor *sparkle_hidden_color2 = gegl_color_new ("#ff7aff");


  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");

  color    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);

 colorerase = gegl_node_new_child (gegl,
                                    "operation", "gimp:layer-mode", "layer-mode", 57,  "blend-space", 1, NULL);

/*
Note to future devs, I am fully aware of GEGL Color to Alpha, for some reason it doesn't do the job as good as Gimp's color erase. It leaves pink artifact.
If you change this to gegl:color-to-alpha it will ruin the filter.
 */

 divide = gegl_node_new_child (gegl,
                                    "operation", "gimp:layer-mode", "layer-mode", 41,  "blend-space", 1, "opacity", 0.03, NULL);
/*
GEGL's divide blend mode doesn't have a built in opacity. Gimp's does thus Gimp's wins.
 */


  color2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                   "value", sparkle_hidden_color, NULL);


  color3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                   "value", sparkle_hidden_color2, NULL);
                           

  opacity   = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);

  ds  = gegl_node_new_child (gegl,
                                  "operation", "gegl:dropshadow",
                                  NULL);


  cellnoise  = gegl_node_new_child (gegl,
                                  "operation", "gegl:cell-noise", "iterations", 1,
                                  NULL);

/*
Crops are bug fixers. They solve the delayed color update bug and allow gaussian blur to be on top of sparkle in a GEGL graph.
 */

  crop   = gegl_node_new_child (gegl,
                                  "operation", "gegl:crop",
                                  NULL);

  content   = gegl_node_new_child (gegl,
                                  "operation", "gegl:src",
                                  NULL);
/*
This, src - is GEGL's equal to the Replace blend mode.
 */


/*
This is an example of a very complex GEGL Graph that features composers inside composers that was made simple by Beaver - via the notes.
 */


/*Everything but crop is inside the replace blend mode. The replace blend mode "content" calls "opacity" the last node in the list. */ 
gegl_node_link_many (input, content, crop, output, NULL);
gegl_node_connect (content, "aux", opacity, "output");
/* Majority of the GEGL Graph is here. Divide and Color Erase are composers AKA virtual layers that are inside a composer "Replace".  */ 
gegl_node_link_many (color2, divide, colorerase, color, ds, opacity, NULL);
/* A Cell Noise is inside the divide blend mode composer*/
gegl_node_connect (divide, "aux", cellnoise, "output"); 
/* Another color fill is inside the color erase blend mode composer all by itself*/
gegl_node_connect (colorerase, "aux", color3, "output");


  gegl_operation_meta_redirect (operation, "color", color, "value");
  gegl_operation_meta_redirect (operation, "opacity", ds, "opacity");
  gegl_operation_meta_redirect (operation, "x", ds, "x");
  gegl_operation_meta_redirect (operation, "y", ds, "y");
  gegl_operation_meta_redirect (operation, "colorshadow", ds, "color");
  gegl_operation_meta_redirect (operation, "scale", cellnoise, "scale");
  gegl_operation_meta_redirect (operation, "shape", cellnoise, "shape");
  gegl_operation_meta_redirect (operation, "seed", cellnoise, "seed");
  gegl_operation_meta_redirect (operation, "opacitymeter", opacity, "value");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:sparkle",
    "title",       _("Sparkle"),
    "reference-hash", "bkagspakzaz10aavx45421xc255001b2ac",
    "description", _("GEGL sparkle effect - works best on 16 bit integer canvases and bright colors"
                     ""),
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Sparkle Effect..."),
    NULL);
}

#endif
