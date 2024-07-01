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
 * 2023 Beaver Sparkle revision
 */

/*
This filter is a modification of beaver:sparkle (GEGL Sparkle) intended for Gimp 2.99. unlike GEGL Sparkle it is 100% GEGL and does not depend on Gimp only operations. 

Rough GEGL Graph of GEGL Stars so users can test it without installing. 

src aux=[ 
color value=#ff7aff
id=opacity over aux=[ ref=opacity divide aux=[  cell-noise shape=1 scale=0.50  ] opacity value=0.05 ]
 ]

rgb-clip color-to-alpha color=#ff7aff median-blur radius=0 crop

color-overlay value=#ffffff
dropshadow radius=2
 */


#define EMBEDDED_GRAPH \
" rgb-clip  color-to-alpha color=#ff7aff   color-overlay value=#ffffff "\
/* This GEGL Graph that is embedded. It saves time so I don't have to list all four of these operations manually */


#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_color (color, _("Color"), "#ffffff")
    description (_("The main sparkle's color"))

property_color  (colorshadow, _("Shadow Color"), "#74e3ff")
  description   (_("The main sparkle's optional shadow color"))



property_double (opacity, _("Shadow clone mode opacity --SLIDE TO ENABLE SHADOW"), 0.0)
  description   (_("A dropshadow that creates the apperance of a second sparkle field. It depends on the sparkle above as it is its drop shadow."))
  value_range   (0.0, 1.5)
  ui_steps      (0.1, 0.10)


property_double  (scale, _("Scale "), 0.24)
    description  (_("The scale of the noise function. (best at 0.10-0.40s) Lower is larger The internal cell noise powering the sparkle."))
  ui_range      (0.1, 0.7)
    ui_range  (0, 1)

property_double  (shape, _("Shape"), 1.0)
    description  (_("Shape transition of the sparkle From a Diamond to a Circle"))
    value_range  (1.0, 2.0)

property_seed    (seed, _("Random seed"), rand)
    description  (_("The random seed for the noise function"))

property_double (masteropacity, _("Hyper Opacity"), 0.04)
  description   (_("Opacity intensity of the stars"))
  ui_range   (0.03, 0.09)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     sparkle2
#define GEGL_OP_C_SOURCE sparkle2.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *output, *color, *normal, *fix, *id1, *dt, *divide, *idref, *sparklestring, *replace, *opacity, *colorchange, *cellnoise, *ds;
  GeglColor *sparkle_embedded_color = gegl_color_new ("#ff7aff");



  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");


 fix    = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur", "radius", 0, "abyss-policy",     GEGL_ABYSS_NONE,
                                  NULL);


  dt   = gegl_node_new_child (gegl,
                                  "operation", "gegl:distance-transform",
                                  NULL);

  id1   = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);

  ds   = gegl_node_new_child (gegl,
                                  "operation", "gegl:dropshadow",
                                  NULL);


  divide   = gegl_node_new_child (gegl,
                                  "operation", "gegl:divide",
                                  NULL);
/*
GEGL's divide blend mode doesn't have a built in opacity. Gimp's does thus Gimp's wins. However it is still possible to make this GEGL only
 */

  opacity   = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity", "value", 0.03,
                                  NULL);
/*
Divide's opacity is 0.03. This is intentional and critical to the filter working. If one wants they can experiment making it go from 0.03 to 0.09
 */

  sparklestring    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl",
                                   "string", EMBEDDED_GRAPH, NULL);
/*
This is an embedded GEGL Graph
 */

  color    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                   "value", sparkle_embedded_color, NULL);


  colorchange  = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);

  cellnoise  = gegl_node_new_child (gegl,
                                  "operation", "gegl:cell-noise", "iterations", 1,
                                  NULL);

  replace   = gegl_node_new_child (gegl,
                                  "operation", "gegl:src",
                                  NULL);

  normal   = gegl_node_new_child (gegl,
                                  "operation", "gegl:over",
                                  NULL);

  idref   = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);

  opacity   = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);


gegl_node_link_many (input, id1, replace, sparklestring, colorchange, fix, ds, output,  NULL);
/*replace blend mode has a lot inside it. following by the rest of the graph that is applied after replace.*/
gegl_node_connect (replace, "aux", normal, "output");
gegl_node_link_many (id1, dt, color, idref, normal, NULL);
/*the replace blend mode has content in it including the normal blend mode, and a id/ref represented as nop.*/
gegl_node_link_many (idref, divide, opacity,  NULL);
gegl_node_connect (normal, "aux", opacity, "output");
/*the normal blend mode inside replace has a divide blend mode inside it. replace->normal->divide*/
gegl_node_connect (divide, "aux", cellnoise, "output");
/*cell noise is inside the divide blend mode*/

/*
src aux=[ 
color value=#ff7aff

id=opacity over aux=[ ref=opacity divide aux=[  cell-noise shape=1 scale=0.50  ] opacity value=0.06 ]
 ]

rgb-clip color-to-alpha color=#ff7aff median-blur radius=0 crop

color-overlay value=#ffffff
dropshadow radius=2*/


  gegl_operation_meta_redirect (operation, "masteropacity", opacity, "value");
  gegl_operation_meta_redirect (operation, "color", colorchange, "value");
  gegl_operation_meta_redirect (operation, "opacity", ds, "opacity");
  gegl_operation_meta_redirect (operation, "colorshadow", ds, "color");
  gegl_operation_meta_redirect (operation, "scale", cellnoise, "scale");
  gegl_operation_meta_redirect (operation, "shape", cellnoise, "shape");
  gegl_operation_meta_redirect (operation, "seed", cellnoise, "seed");

}
static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:sparkle2",
    "title",       _("Sparkle (revision)"),
    "reference-hash", "bspgsaakzaz55aavsp54sp0xc25500f2ec",
    "description", _("Render Sparkles on Canvas - works best on 16 bit integer canvases and bright colors"
                     ""),
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Sparkle Effect 2..."),
    NULL);
}

#endif
