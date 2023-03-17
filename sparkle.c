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

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_color (color3, _("Color 3"), "#ff7aff")
    description (_("The color to paint over the input"))
    ui_meta     ("role", "output-extent")

property_color (color2, _("Color 2"), "#ff7aff")
    description (_("The color to paint over the input"))
    ui_meta     ("role", "output-extent")

property_color (color, _("Color"), "#ffffff")
    description (_("The color to paint over the input"))


property_color  (colorshadow, _("Shadow Color"), "#74e3ff")
    /* TRANSLATORS: the string 'black' should not be translated */
  description   (_("The shadow's color (defaults to 'black')"))

/* It does make sense to sometimes have opacities > 1 (see GEGL logo
 * for example)
 */


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



property_double (opacity, _("Shadow clone mode Opacity --SLIDE TO ENABLE SHADOW"), 0.0)
  value_range   (0.0, 1.5)
  ui_steps      (0.1, 0.10)


property_file_path(upload, _("Top image Overlay"), "")
    description (_("Source image file path (png, jpg, raw, svg, bmp, tif, ...)"))
    ui_meta     ("role", "output-extent")


property_double  (scale, _("Scale (best at 0.10-0.40s) Lower is larger"), 0.14)
    description  (_("The scale of the noise function"))
    value_range  (0, 1)

property_double  (shape, _("Shape"), 1.0)
    description  (_("Interpolate between Manhattan and Euclidean distance."))
    value_range  (1.0, 2.0)



property_int     (iterations, _("Iterations"), 1)
    description  (_("The number of noise octaves."))
    value_range  (1, 20)
    ui_meta     ("role", "output-extent")



property_seed    (seed, _("Random seed"), rand)
    description  (_("The random seed for the noise function"))

property_double (opacitymeter, _("Above 100% Opacity Meter"), 6)
  value_range   (0, 6.0)
  ui_steps      (1.0, 6.0)




#else

#define GEGL_OP_META
#define GEGL_OP_NAME     sparkle
#define GEGL_OP_C_SOURCE sparkle.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *output, *ontop, *color, *divide, *colorerase, *opacity, *layer, *color2, *color3, *cellnoise, *ds;


  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");

  color    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);

 colorerase = gegl_node_new_child (gegl,
                                    "operation", "gimp:layer-mode", "layer-mode", 57,  "blend-space", 1, NULL);

 divide = gegl_node_new_child (gegl,
                                    "operation", "gimp:layer-mode", "layer-mode", 41,  "blend-space", 1, "opacity", 0.03, NULL);


  color2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                  NULL);

  color3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                  NULL);


  ontop   = gegl_node_new_child (gegl,
                                  "operation", "gegl:src-atop",
                                  NULL);

  opacity   = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);

  ds  = gegl_node_new_child (gegl,
                                  "operation", "gegl:dropshadow",
                                  NULL);

  layer  = gegl_node_new_child (gegl,
                                  "operation", "gegl:layer",
                                  NULL);

  cellnoise  = gegl_node_new_child (gegl,
                                  "operation", "gegl:cell-noise",
                                  NULL);









 gegl_node_link_many (input, color2, divide, colorerase, color, ds, opacity, ontop, output, NULL);
gegl_node_connect_from (divide, "aux", cellnoise, "output"); 
gegl_node_connect_from (colorerase, "aux", color3, "output"); 
gegl_node_connect_from (ontop, "aux", layer, "output"); 




  gegl_operation_meta_redirect (operation, "color", color, "value");
  gegl_operation_meta_redirect (operation, "color3", color3, "value");
  gegl_operation_meta_redirect (operation, "upload", layer, "src");
  gegl_operation_meta_redirect (operation, "opacity", ds, "opacity");
  gegl_operation_meta_redirect (operation, "x", ds, "x");
  gegl_operation_meta_redirect (operation, "y", ds, "y");
  gegl_operation_meta_redirect (operation, "colorshadow", ds, "color");
  gegl_operation_meta_redirect (operation, "color2", color2, "value");
  gegl_operation_meta_redirect (operation, "scale", cellnoise, "scale");
  gegl_operation_meta_redirect (operation, "shape", cellnoise, "shape");
  gegl_operation_meta_redirect (operation, "seed", cellnoise, "seed");
  gegl_operation_meta_redirect (operation, "iterations", cellnoise, "iterations");
  gegl_operation_meta_redirect (operation, "opacitymeter", opacity, "value");






}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "gegl:sparkle",
    "title",       _("Sparkle"),
    "categories",  "Artistic",
    "reference-hash", "bkagspakzaz10aavx45421xc255001b2ac",
    "description", _("GEGL sparkle effect - works best on 16 bit integer canvases and bright colors"
                     ""),
    NULL);
}

#endif
