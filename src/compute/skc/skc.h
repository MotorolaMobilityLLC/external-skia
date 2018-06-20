/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SKC
#define SKC_ONCE_SKC

//
// FIXME -- get rid of these here
//

#include <stdint.h>
#include <stdbool.h>

//
//
//

#include "skc_styling.h" // FIXME -- skc_styling
// #include "skc_err.h"

//
// FIXME -- move errors to an skc prefixed include
//

typedef enum skc_err {

  SKC_ERR_SUCCESS                           = 0,

  SKC_ERR_API_BASE                          = 10000,

  SKC_ERR_NOT_IMPLEMENTED                   = SKC_ERR_API_BASE,

  SKC_ERR_POOL_EMPTY,

  SKC_ERR_CONDVAR_WAIT,

  SKC_ERR_LAYER_ID_INVALID,
  SKC_ERR_LAYER_NOT_EMPTY,

  SKC_ERR_TRANSFORM_WEAKREF_INVALID,
  SKC_ERR_STROKE_STYLE_WEAKREF_INVALID,

  SKC_ERR_COMMAND_NOT_READY,
  SKC_ERR_COMMAND_NOT_COMPLETED,
  SKC_ERR_COMMAND_NOT_STARTED,

  SKC_ERR_COMMAND_NOT_READY_OR_COMPLETED,

  SKC_ERR_COMPOSITION_SEALED,
  SKC_ERR_STYLING_SEALED,

  SKC_ERR_HANDLE_INVALID,
  SKC_ERR_HANDLE_OVERFLOW,

  SKC_ERR_COUNT

} skc_err;

//
// SPINEL TYPES
//

typedef struct skc_context          * skc_context_t;
typedef struct skc_path_builder     * skc_path_builder_t;
typedef struct skc_raster_builder   * skc_raster_builder_t;

typedef struct skc_composition      * skc_composition_t;
typedef struct skc_styling          * skc_styling_t;

typedef struct skc_surface          * skc_surface_t;

#if 0
typedef struct skc_interop          * skc_interop_t;
typedef        uint32_t               skc_interop_surface_t;
#endif

typedef        uint32_t               skc_path_t;
typedef        uint32_t               skc_raster_t;

typedef        uint32_t               skc_layer_id;
typedef        uint32_t               skc_group_id;

typedef        uint32_t               skc_styling_cmd_t;

typedef        uint64_t               skc_weakref_t;
typedef        skc_weakref_t          skc_transform_weakref_t;
typedef        skc_weakref_t          skc_raster_clip_weakref_t;

//
// FIXME -- bury all of this
//

#define SKC_STYLING_CMDS(...) _countof(__VA_ARGS__),__VA_ARGS__
#define SKC_GROUP_IDS(...)    _countof(__VA_ARGS__),__VA_ARGS__

//
//
//

#define SKC_PATH_INVALID     UINT32_MAX
#define SKC_RASTER_INVALID   UINT32_MAX
#define SKC_WEAKREF_INVALID  UINT64_MAX

//
// TRANSFORM LAYOUT: { sx shx tx shy sy ty w0 w1 }
//

extern float const * const skc_transform_identity_ptr; // { 1, 0, 0, 0, 1, 0, 0, 0 }

//
// RASTER CLIP LAYOUT: { x0, y0, x1, y1 }
//

extern float const * const skc_raster_clip_default_ptr;

//
// CONTEXT
//

skc_err
skc_context_create(skc_context_t       * context,
                   char          const * target_platform_substring,
                   char          const * target_device_substring,
                   intptr_t              context_properties[]);

skc_err
skc_context_retain(skc_context_t context);

skc_err
skc_context_release(skc_context_t context);

skc_err
skc_context_reset(skc_context_t context);

//
// COORDINATED EXTERNAL OPERATIONS
//

/*
  Examples include:

  - Transforming an intermediate layer with a blur, sharpen, rotation or scaling kernel.
  - Subpixel antialiasing using neighboring pixel color and coverage data.
  - Performing a blit from one region to another region on a surface.
  - Blitting from one surface to another.
  - Loading and processing from one region and storing to another region.
  - Rendezvousing with an external pipeline.
*/

//
//
//

bool
skc_context_yield(skc_context_t context);

void
skc_context_wait(skc_context_t context);

//
// PATH BUILDER
//

skc_err
skc_path_builder_create(skc_context_t context, skc_path_builder_t * path_builder);

skc_err
skc_path_builder_retain(skc_path_builder_t path_builder);

skc_err
skc_path_builder_release(skc_path_builder_t path_builder);

//
// PATH OPS
//

skc_err
skc_path_begin(skc_path_builder_t path_builder);

skc_err
skc_path_end(skc_path_builder_t path_builder, skc_path_t * path);

skc_err
skc_path_retain(skc_context_t context, skc_path_t const * paths, uint32_t count);

skc_err
skc_path_release(skc_context_t context, skc_path_t const * paths, uint32_t count);

skc_err
skc_path_flush(skc_context_t context, skc_path_t const * paths, uint32_t count);

//
// PATH SEGMENT OPS
//

//
// FIXME -- we need a bulk/vectorized path segment operation
//

skc_err
skc_path_move_to(skc_path_builder_t path_builder,
                 float x0, float y0);

skc_err
skc_path_close(skc_path_builder_t path_builder);

skc_err
skc_path_line_to(skc_path_builder_t path_builder,
                 float x1, float y1);

skc_err
skc_path_cubic_to(skc_path_builder_t path_builder,
                  float x1, float y1,
                  float x2, float y2,
                  float x3, float y3);

skc_err
skc_path_cubic_smooth_to(skc_path_builder_t path_builder,
                         float x2, float y2,
                         float x3, float y3);

skc_err
skc_path_quad_to(skc_path_builder_t path_builder,
                 float x1, float y1,
                 float x2, float y2);

skc_err
skc_path_quad_smooth_to(skc_path_builder_t path_builder,
                        float x2, float y2);

skc_err
skc_path_ellipse(skc_path_builder_t path_builder,
                 float cx, float cy,
                 float rx, float ry);

//
// RASTER BUILDER
//

skc_err
skc_raster_builder_create(skc_context_t context, skc_raster_builder_t * raster_builder);

skc_err
skc_raster_builder_retain(skc_raster_builder_t raster_builder);

skc_err
skc_raster_builder_release(skc_raster_builder_t raster_builder);

//
// RASTER OPS
//

skc_err
skc_raster_begin(skc_raster_builder_t raster_builder);

skc_err
skc_raster_end(skc_raster_builder_t raster_builder, skc_raster_t * raster);

skc_err
skc_raster_retain(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

skc_err
skc_raster_release(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

skc_err
skc_raster_flush(skc_context_t context, skc_raster_t const * rasters, uint32_t count);

//
// PATH-TO-RASTER OPS
//

//
// FIXME -- do we need a bulk/vectorized "add filled" function?
//

skc_err
skc_raster_add_filled(skc_raster_builder_t        raster_builder,
                      skc_path_t                  path,
                      skc_transform_weakref_t   * transform_weakref,
                      float               const * transform,
                      skc_raster_clip_weakref_t * raster_clip_weakref,
                      float               const * raster_clip);

//
// COMPOSITION STATE
//

skc_err
skc_composition_create(skc_context_t context, skc_composition_t * composition);

skc_err
skc_composition_retain(skc_composition_t composition);

skc_err
skc_composition_release(skc_composition_t composition);

skc_err
skc_composition_place(skc_composition_t    composition,
                      skc_raster_t const * rasters,
                      skc_layer_id const * layer_ids,
                      float        const * txs,
                      float        const * tys,
                      uint32_t             count); // NOTE: A PER-PLACE CLIP IS POSSIBLE

skc_err
skc_composition_seal(skc_composition_t composition);

skc_err
skc_composition_unseal(skc_composition_t composition, bool reset);

skc_err
skc_composition_get_bounds(skc_composition_t composition, int32_t bounds[4]);

#if 0
// let's switch to a per place bounds using weakrefs -- clip 0 will be largest clip
skc_err
skc_composition_set_clip(skc_composition_t composition, int32_t const clip[4]);
#endif

//
// TODO: COMPOSITION "SET ALGEBRA" OPERATIONS
//
// Produce a new composition from the union or intersection of two
// existing compositions
//

//
// TODO: COMPOSITION "HIT DETECTION"
//
// Report which layers and tiles are intersected by one or more
// device-space (x,y) points
//

//
// STYLING STATE
//

skc_err
skc_styling_create(skc_context_t   context,
                   skc_styling_t * styling,
                   uint32_t        layers_count,
                   uint32_t        groups_count,
                   uint32_t        extras_count);

skc_err
skc_styling_retain(skc_styling_t styling);

skc_err
skc_styling_release(skc_styling_t styling);

skc_err
skc_styling_seal(skc_styling_t styling);

skc_err
skc_styling_unseal(skc_styling_t styling); // FIXME

skc_err
skc_styling_reset(skc_styling_t styling); // FIXME -- make unseal reset

//
// STYLING GROUPS AND LAYERS
//

skc_err
skc_styling_group_alloc(skc_styling_t  styling,
                        skc_group_id * group_id);

skc_err
skc_styling_group_enter(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);

skc_err
skc_styling_group_leave(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);                        

skc_err
skc_styling_group_parents(skc_styling_t        styling,
                          skc_group_id         group_id,
                          uint32_t             depth,
                          skc_group_id const * parents);

skc_err
skc_styling_group_range_lo(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_lo);

skc_err
skc_styling_group_range_hi(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_hi);

skc_err
skc_styling_group_layer(skc_styling_t             styling,
                        skc_group_id              group_id,
                        skc_layer_id              layer_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds);                        

//
// STYLING ENCODERS -- FIXME -- WILL EVENTUALLY BE OPAQUE
//

void
skc_styling_layer_fill_rgba_encoder(skc_styling_cmd_t * cmds, float const rgba[4]);

void
skc_styling_background_over_encoder(skc_styling_cmd_t * cmds, float const rgba[4]);

void
skc_styling_layer_fill_gradient_encoder(skc_styling_cmd_t         * cmds,
                                        float                       x0,
                                        float                       y0,
                                        float                       x1,
                                        float                       y1,
                                        skc_styling_gradient_type_e type,
                                        uint32_t                    n,
                                        float                 const stops[],
                                        float                 const colors[]);

//
// SURFACE
//

//
// FIXME - surface create needs to be able to specify different
// surface targets here that are a function of the surface type and
// rendering model: CL/global, GL/buffer, simple SRCOVER model,
// complex group-based PDF rendering model, etc.
//

skc_err
skc_surface_create(skc_context_t context, skc_surface_t * surface);

skc_err
skc_surface_retain(skc_surface_t surface);

skc_err
skc_surface_release(skc_surface_t surface);

// skc_interop_surface_t
// skc_surface_interop_surface_get(skc_surface_t surface);

//
// NO NO NO -- SKC will always be a client of some other platform so
// handle things like blits and clears there unless it's something
// unique like an SKC tile-based clear/blit.
//
// (temporarily implement these for testing porpoises)
//

skc_err
skc_surface_clear(skc_surface_t  surface, 
                  float    const rgba[4], 
                  uint32_t const rect[4],
                  void         * fb);

skc_err
skc_surface_blit(skc_surface_t  surface, 
                 uint32_t const rect[4], 
                 int32_t  const txty[2]);

//
// SURFACE RENDER
//

typedef void (*skc_surface_render_pfn_notify)(skc_surface_t     surface,
                                              skc_styling_t     styling,
                                              skc_composition_t composition,
                                              void            * data);
skc_err
skc_surface_render(skc_surface_t                 surface,
                   uint32_t                const clip[4],
                   skc_styling_t                 styling,
                   skc_composition_t             composition,
                   skc_surface_render_pfn_notify notify,
                   void                        * data,
                   void                        * fb); // FIXME FIXME

//
//
//

#endif

//
//
//

