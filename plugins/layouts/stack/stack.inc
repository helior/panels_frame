<?php

/**
 * @todo Find out why Layout changer is broken with Stack.
 */

$plugin = array(
  'title' => t('Frame: Stack'),
  'category' => t('Builders'),
  'icon' => 'envelope.png',
  'builder' => TRUE,
  'builder tab title' => 'Stack this mother effers!',
  // 'get child' => 'panels_frame_stack_get_sublayout',
  // 'get children' => 'panels_frame_stack_get_sublayouts',
  'regions function' => 'panels_frame_stack_region',
  'theme' => 'panels_frame_stack',
  'admin css' => 'stack.admin.css',
  'settings form' => 'panels_frame_stack_settings',
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

function panels_frame_stack_fake_frames() {
  return array(
    'first' => array(
      'layout' => 'twocol',
    ),
    'second' => array(
      'layout' => 'threecol_25_50_25',
    ),
    'third' => array(
      'layout' => 'tester',
    ),
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
 * Settings form builder.
 */
function panels_frame_stack_settings(&$display, $layout, $settings) {
  foreach ($display->panels_frame as $name => $frame) {
    if (!empty($frame['plugin']['settings form']) && function_exists($frame['plugin']['settings form'])) {
      $form[$name] = array(
        '#type' => 'fieldset',
        '#title' => t('@label settings', array('@label' => $frame['plugin']['title'] . ' (' . $name . ')')),
      ) + $frame['plugin']['settings form']($display, $frame['plugin'], isset($settings[$name]) ? $settings[$name] : array());
    }
  }

  return $form;
}

/**
 * Regions callback.
 * Collect the regions from the compiled layouts.
 */
function panels_frame_stack_region(&$display, $settings, $base_layout) {
  $regions = array();

  // @todo replace with export_ui objects
  foreach (panels_frame_stack_fake_frames() as $name => $frame) {
    // Avoid recursion. @todo add form validation to prevent this from occurring on the front-end.
    if ($display->layout == $frame['layout']) {
      watchdog('panels_frame', '@frame cannot be embedded within itself!', array('@frame' => $frame['layout']), WATHDOG_WARNING);
      continue;
    }

    $frame['plugin'] = panels_get_layout($frame['layout']);

    foreach (panels_get_regions($frame['plugin'], $display) as $region => $label) {
      $frame['regions'][$region]['region'] = $frame['plugin']['regions'][$region] = $label;
      $regions[$name . '__' . $region] = $label;
    }

    $display->panels_frame[$name] = $frame;
  }

  return $regions;
}

/**
 * Implements template_preprocess_HOOK().
 * @see theme_panels_frame_stack().
 */
function template_preprocess_panels_frame_stack(&$vars) {
  panels_frame_stack_distribute_layout_content($vars);

  $vars['attributes_array']['id'] = $vars['css_id'];
  $vars['attributes_array']['class'][] = 'panels-frame';
}

/**
 * Implements template_process_HOOK().
 * @see theme_panels_frame_stack().
 */

function template_process_panels_frame_stack(&$vars) {
  $vars['attributes'] = drupal_attributes($vars['attributes_array']);
}

/**
 * Theme callback for stack layout.
 */
function theme_panels_frame_stack($vars) {
  $output = '';

  foreach ($vars['frames'] as $name => $content) {
    $output .= '<div class="frame clearfix frame-' . $name . '">' . $content . '</div>';
  }

  return '<div' . $vars['attributes'] . '>' . $output . '</div>';
  // return "<div{$vars['attributes']}>$output</div>";
}

function panels_frame_stack_distribute_layout_content(&$vars) {
  // Back-up original renderer prefix for the encompassing layout render
  $renderer_prefix = $vars['renderer']->prefix;
  $renderer_layout = $vars['renderer']->plugins['layout'];

  foreach ($vars['display']->panels_frame as $name => $frame) {
    // Distribute content
    $content = array();
    foreach (array_keys($frame['regions']) as $region) {
      $key = $name . '__' . $region;
      $content[$region] = $vars['content'][$key];
    }

    // Choose theme
    if ($vars['renderer']->admin && !empty($frame['plugin']['admin theme'])) {
      $theme = $frame['plugin']['admin theme'];
    }
    else {
      $theme = $frame['plugin']['theme'];
    }

    $vars['renderer']->prefix = '';
    $vars['renderer']->plugins['layout'] = $frame['plugin'];

    $vars['renderer']->add_meta();
    $settings = isset($vars['display']->layout_settings[$name]) ? $vars['display']->layout_settings[$name] : array();
    $vars['frames'][$name] = $vars['renderer']->prefix . theme($theme, array('content' => $content, 'settings' => $settings, 'display' => $vars['display'], 'layout' => $frame['plugin'], 'renderer' => $vars['renderer']));
  }

  $vars['renderer']->prefix = $renderer_prefix;
  $vars['renderer']->plugins['layout'] = $renderer_layout;
}
