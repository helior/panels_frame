<?php

$plugin = array(
  'title' => t('Tester'),
  'css' => 'tester.css',
  'admin css' => 'tester.admin.css',
  'regions function' => 'panels_frame_tester_regions',
  'theme' => 'panels_frame_tester',
  'settings form' => 'panels_frame_tester_settings',
  'icon' => 'hong-kong.gif',
);

function panels_frame_tester_settings(&$display, $layout, $settings) {
  $form['test'] = array(
    '#type' => 'textfield',
    '#title' => 'test',
    '#default_value' => isset($settings['test']) ? $settings['test'] : '',
  );
  return $form;
}

function panels_frame_tester_regions(&$display, $settings, $layout) {
  return array(
    'top' => 'Top',
    'bottom' => 'Bottom',
  );
}

function theme_panels_frame_tester($vars) {
  $output = '<div class="tester">';

  foreach ($vars['layout']['regions'] as $name => $label) {
    $output .= '<div class="region-' . $name . '">' . $vars['content'][$name] . '</div>';
  }

  $output .= '</div>';

  return $output;
}