<?php

/**
 * Ajax menu callback to add a fram to a Panels Frame Stack.
 */
function panels_frame_ajax_stack_frame_add() {
  ctools_include('ajax');
  ctools_include('modal');

  $form_state = array(
    'title' => t('Add Frame'),
    'ajax' => TRUE,
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
  ctools_include('common', 'panels');
  ctools_include('display-layout', 'panels');
  ctools_include('plugins', 'panels');

  // @todo -- figure out where/how to deal with this.
  $form_state['allowed_layouts'] = 'panels_frame';

  // Obligitory Display object
  $form_state['display'] = panels_new_display();

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';

  $form = panels_choose_layout($form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

function panels_frame_stack_frame_add_validate(&$form, &$form_state) {
  // dpm($form_state,'state');
  if (empty($form_state['values']['layout'])) {
    form_set_error('layout', 'wtf man?');
  }
}