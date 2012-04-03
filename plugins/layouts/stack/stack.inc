<?php

$plugin = array(
  'title' => t('Frame: Stack'),
  'category' => t('Builders'),
  'icon' => 'envelope.png',
  // 'builder' => TRUE,
  // 'builder tab title' => 'Stack this mother effers!',
  // 'get child' => 'panels_frame_stack_get_sublayout',
  // 'get children' => 'panels_frame_stack_get_sublayouts',
  'regions function' => 'panels_frame_stack_region',
  'theme' => 'panels_frame_stack',
);

// function panels_frame_stack_fake_layout($name, $plugin) {
//   return array(
//     'name' => $name,
//     'category' => 'Frame: Stack',
//     'title' => ucfirst($name),
//     'description' => $name . ' is a "Stack" instance',
//     'builder' => FALSE,
//     'builder tabe title' => NULL,
//   ) + $plugin;
// }

function panels_frame_stack_fake_configured_layouts() {
  return array(
    'twocol',
    'twocol_bricks',
    // 'stack',
  );
}

// function panels_frame_stack_get_sublayout($plugin, $layout_name, $sublayout_name) {
//   // @todo this should load from Ctools Export/CRUD API
//   return panels_frame_stack_fake_layout('bob', $plugin);
// }
//
// function panels_frame_stack_get_sublayouts($plugin, $layout_name) {
//   // @todo this should load from Ctools Export/CRUD API
//   $sub_layouts = array(
//     'bob' => panels_frame_stack_fake_layout('bob', $plugin),
//     'steve' => panels_frame_stack_fake_layout('steve', $plugin),
//     'bill' => panels_frame_stack_fake_layout('bill', $plugin),
//   );
//   return $sub_layouts;
// }

/**
 * Regions callback.
 * Collect the regions from the compiled layouts.
 * @todo layouts should be derived from configured sub-layouts. They should be
 * placed in the top-level of the array with unique keys (which should be abled
 * to be mapped to the original layout)
 * @todo detect recursion, in particular that we don't have itself as a sub-layout
 */
function panels_frame_stack_region(&$display, $settings, $layout) {
  $regions = array();
  foreach (panels_frame_stack_fake_configured_layouts() as $sub_layout) {
    // Avoid recursion.
    if ($display->layout == $sub_layout) {
      continue;
    }
    $sub_layout_plugin = panels_get_layout($sub_layout);
    $regions += panels_get_regions($sub_layout_plugin, $display);
  }
  return $regions;
}

/**
 * @todo render each individual sub-layout; map content to appropriate regions.
 */
function template_preprocess_panels_frame_stack(&$vars) {}

function theme_panels_frame_stack($vars) {
  $o  = '<div>';
  $o .=   '<div class="leeft">' . $vars['content']['leeft'] . '</div>';
  $o .=   '<div class="riight">' . $vars['content']['riight'] . '</div>';
  $o .= '</div>';
  return $o;
}