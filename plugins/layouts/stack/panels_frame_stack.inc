<?php

$plugin = array(
  'title' => t('Panels Frame Stack'),
  'category' => t('Builders'),
  'icon' => 'stack.png',
  'get child' => 'panels_frame_stack_get_sublayout',
  'get children' => 'panels_frame_stack_get_sublayouts',
  'regions function' => 'panels_frame_stack_region',
  'theme' => 'panels_frame_stack',
  'admin css' => 'stack.admin.css',
  'settings form' => 'panels_frame_stack_settings',
);

/**
 * Get child callback.
 */
function panels_frame_stack_get_sublayout($plugin, $layout_name, $sublayout_name) {
  ctools_include('export');
  if ($item = ctools_export_crud_load('panels_frame', $sublayout_name)) {
    return panels_frame_stack_child_plugin($plugin, $item);
  }
}

/**
 * Get children callback.
 */
function panels_frame_stack_get_sublayouts($plugin, $layout_name) {
  ctools_include('export');
  $items = ctools_export_load_object('panels_frame', 'conditions', array('plugin' => 'stack'));
  $layouts = array();
  foreach ($items as $name => $item) {
    $layouts['panels_frame_stack:' . $name] = panels_frame_stack_child_plugin($plugin, $item);
  }

  return $layouts;
}

/**
 * Helper to generate the layout definition for sub-layouts.
 */
function panels_frame_stack_child_plugin($plugin, $object) {
  $plugin['title'] = check_plain($object->label);
  $plugin['category'] = empty($object->category) ? t('Miscellaneous') : check_plain($object->category);
  $plugin['description'] = check_plain($object->description);
  $plugin['panels_frame'] = $object;
  unset($plugin['builder'], $plugin['get child'], $plugin['get children']);

  return $plugin;
}

/**
 * Settings form builder.
 */
function panels_frame_stack_settings(&$display, $layout, $settings) {
  $form = array();
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
  foreach ($base_layout['panels_frame']->data as $name => $frame) {
    if ($display->layout == $frame['layout']) {
      watchdog('panels_frame', 'Uh-oh. @frame cannot be embedded within itself! !link', array('@frame' => $frame['layout'], '!link' => l(t('Fix the anomaly'), 'admin/structure/panels/frame/stacks/list/' . $frame['name'] . '/edit/frames')), WATHDOG_WARNING);
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
}

function panels_frame_stack_distribute_layout_content(&$vars) {
  // Back-up original renderer prefix/suffix for the encompassing layout render
  $renderer_prefix = $vars['renderer']->prefix;
  $renderer_suffix = $vars['renderer']->suffix;
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
    $vars['renderer']->suffix = '';
    $vars['renderer']->plugins['layout'] = $frame['plugin'];

    $vars['renderer']->add_meta();
    $settings = isset($vars['display']->layout_settings[$name]) ? $vars['display']->layout_settings[$name] : array();

    $vars['frames'][$name]  = $vars['renderer']->prefix;
    $vars['frames'][$name] .= theme($theme, array('content' => $content, 'settings' => $settings, 'display' => $vars['display'], 'layout' => $frame['plugin'], 'renderer' => $vars['renderer']));
    $vars['frames'][$name] .= $vars['renderer']->suffix;
  }

  $vars['renderer']->prefix = $renderer_prefix;
  $vars['renderer']->plugins['layout'] = $renderer_layout;
}
