<?php

/**
 * Ajax menu callback to add a fram to a Panels Frame Stack.
 */
function panels_frame_ajax_stack_frame_add($cache_mechanism, $cache_key, $name = NULL) {
  ctools_include('ajax');
  ctools_include('cache');
  ctools_include('modal');

  if (empty($cache_mechanism) || empty($cache_key)) {
    ctools_ajax_render_error();
  }

  // Load stored object from cache.
  if (!($object = ctools_cache_get($cache_mechanism, $cache_key))) {
    ctools_ajax_render_error(t('Invalid object name.'));
  }

  $form_state = array(
    'title' => t('Add Frame'),
    'ajax' => TRUE,
    'cache_mechanism' => $cache_mechanism,
    'cache_key' => $cache_key,
    'object' => $object,
  );

  $commands = ctools_modal_form_wrapper('panels_frame_stack_frame_add', $form_state);

  if ($form_state['executed']) {
    $commands = array();
    // @todo rebuild table and replace it.
    $commands[] = ctools_modal_command_dismiss();
  }

  print ajax_render($commands);
  exit;
}

function panels_frame_stack_frame_add($form, &$form_state) {
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#required' => TRUE,
    '#size' => 40,
  );

  $form['identifier'] = array(
    '#type' => 'machine_name',
    '#title' => t('Identifier'),
    '#size' => 40,
    '#machine_name' => array(
      'exists' => 'panels_frame_frame_identifier_exists',
      'source' => array('label'),
      'label' => 'identifier',
    ),
  );

  $form['weight'] = array(
    '#type' => 'value',
    '#value' => 0,
  );

  $form['layout'] = array(
    '#title' => 'layout',
    '#type' => 'panels_frame_layout',
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

function panels_frame_stack_frame_add_submit(&$form, &$form_state) {
  $cache_mechanism = $form_state['cache_mechanism'];
  $cache_key = $form_state['cache_key'];
  $object = $form_state['object'];

  foreach (array('label', 'identifier', 'weight', 'layout') as $key) {
    $frame[$key] = $form_state['values'][$key];
  }

  $object->data[$frame['identifier']] = $frame;
  ctools_cache_set($cache_mechanism, $cache_key, $object);
}
