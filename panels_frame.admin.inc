<?php

/**
 * Ajax menu callback to add a fram to a Panels Frame Stack.
 */
function panels_frame_ajax_stack_frame_add($cache_mechanism, $key) {
  ctools_include('cache');
  ctools_include('ajax');
  ctools_include('modal');

  $cache = ctools_cache_get($cache_mechanism, $key);

  $form_state = array(
    'title' => t('Add Frame'),
    'ajax' => TRUE,
    'cache_mechanism' => $cache_mechanism,
    'key' => $key,
    'item' => $cache,
  );

  $commands = ctools_modal_form_wrapper('panels_frame_stack_frame_add', $form_state);

  if ($form_state['executed']) {
    $commands = array();
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

  $form['layout'] = array(
    '#title' => 'layout',
    '#type' => 'panels_frame_choose_layout',
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function panels_frame_choose_layout_form_add_submit(&$form, &$form_state) {
  // $this->edit_cache_set($form_state['item'], $form_state['form type']);
}
