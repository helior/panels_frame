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

  $form = panels_frame_choose_layout_form($form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validation callback for 'panels_frame_stack_frame_add'.
 */
function panels_frame_stack_frame_add_validate(&$form, &$form_state) {
  // dpm($form_state,'state');
  if (empty($form_state['values']['layout'])) {
    form_set_error('layout', 'wtf man?');
  }
}

function panels_frame_choose_layout_form($form, &$form_state) {
  ctools_include('common', 'panels');
  ctools_include('cleanstring');

  $groups = array();
  $id = drupal_html_id('panels-frame-choose-layout');

  $allowed_layouts = $form_state['allowed_layouts'] ? $form_state['allowed_layouts'] : 'panels_frame';
  $layouts = panels_common_get_allowed_layouts($allowed_layouts);

  $groups = array();
  foreach ($layouts as $lid => $layout) {
    $key = ctools_cleanstring($layout['category']);
    $groups[$key]['category'] = $layout['category'];
    $groups[$key]['layouts'][] = $lid;
  }

  $form['layout'] = array(
    '#type' => 'container',
    '#attributes' => array('id' => $id)
  );

  $form['layout']['header'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  foreach ($groups as $name => $group) {
    $key = drupal_html_id($name);
    $form['layout']['header']['#items'][] = '<a href="#' . $key . '">' . $group['category'] . '</a>';
    $form['layout']['content'][$name] = array(
      '#type' => 'container',
      '#attributes' => array('id' => $key),
    );

    foreach ($group['layouts'] as $lid) {
      $form['layout']['content'][$name][$lid] = array(
        '#type' => 'radio',
        '#title' => $layouts[$lid]['title'],
        '#parents' => array('layout'),
        '#id' => drupal_clean_css_identifier('edit-layout-' . $lid),
        '#default_value' => $form_state['values']['layout'] == $lid,
      );
    }
  }

  return $form;
}
