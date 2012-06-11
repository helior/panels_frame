<?php

/**
 * Ajax menu callback to add a fram to a Panels Frame Stack.
 */
function panels_frame_stack_frame_ajax_delegate($op, $cache_mechanism, $cache_key, $name = NULL) {
  ctools_include('ajax');
  ctools_include('cache');
  ctools_include('modal');

  // Fail early if we don't have the data we need.
  if (empty($op) || empty($cache_mechanism) || empty($cache_key)) {
    ctools_ajax_render_error();
  }

  // Load stored object from cache.
  if (!($object = ctools_cache_get($cache_mechanism, $cache_key))) {
    ctools_ajax_render_error(t('Invalid object name.'));
  }

  $form_state = array(
    'title' => t('!op Frame @name', array('!op' => ucwords($op), '@name' => $name)),
    'cache_mechanism' => $cache_mechanism,
    'cache_key' => $cache_key,
    'object' => $object,
    'frame' => $name,
    'ajax' => TRUE,
    'op' => $op,
  );

  $commands = ctools_modal_form_wrapper("panels_frame_stack_frame_$op", $form_state);

  if ($form_state['executed']) {
    $commands = array();
    // @todo render the table-drag correctly and replace on page. Only then can
    // you simply dismiss the modal dialog box. Otherwise, just refresh the page.
    // $commands[] = ctools_modal_command_dismiss();
    $commands[] = ctools_ajax_command_reload();
  }

  print ajax_render($commands);
  exit;
}

/**
 * Helper for building the frame form.
 */
function panels_frame_stack_frame_form(&$form_state) {
  $data = $form_state['object']->data;
  $name = $form_state['frame'];

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => isset($data[$name]['label']) ? $data[$name]['label'] : '',
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
    '#default_value' => isset($data[$name]['identifier']) ? $data[$name]['identifier'] : '',
  );

  $form['weight'] = array(
    '#type' => 'value',
    '#default_value' => isset($data[$name]['weight']) ? $data[$name]['weight'] : 0,
  );

  $form['layout'] = array(
    '#title' => 'layout',
    '#type' => 'panels_frame_layout',
    '#required' => TRUE,
    '#no_builders' => TRUE,
    '#no_frames' => TRUE,
    '#default_value' => isset($data[$name]['layout']) ? $data[$name]['layout'] : '',
  );

  return $form;
}

/**
 * Form builder for "add"-ing a frame.
 */
function panels_frame_stack_frame_add($form, &$form_state) {
  $form = panels_frame_stack_frame_form($form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#validate' => array('panels_frame_stack_frame_layout_validate'),
    '#submit' => array('panels_frame_stack_frame_common_submit'),
  );

  return $form;
}

/**
 * Form builder for "edit"-ing a frame.
 */
function panels_frame_stack_frame_edit($form, &$form_state) {
  $form = panels_frame_stack_frame_form($form_state);
  $form['identifier']['#disabled'] = TRUE;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('panels_frame_stack_frame_layout_validate'),
    '#submit' => array('panels_frame_stack_frame_common_submit'),
  );

  return $form;
}

/**
 * Form builder for "clone"-ing a frame.
 */
function panels_frame_stack_frame_clone($form, &$form_state) {
  $form = panels_frame_stack_frame_form($form_state);
  $form['label']['#default_value'] .= ' ' . t('copy');
  unset($form['identifier']['#default_value']);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clone'),
    '#validate' => array('panels_frame_stack_frame_layout_validate'),
    '#submit' => array('panels_frame_stack_frame_common_submit'),
  );

  return $form;
}

/**
 * Form builder for "delete"-ing a frame.
 * @todo create a small, custom JS theme for ctools modal.
 */
function panels_frame_stack_frame_delete($form, &$form_state) {
  $question = t('Are you sure you want to remove the frame @name?', array('@name' => $form_state['frame']));
  $form = confirm_form($form, $question, '');
  $form['description']['#markup'] = $question . ' ' . $form['description']['#markup'];
  unset($form['actions']['cancel']);

  return $form;
}

/**
 * Submission handler for "delete"-ing a frame.
 */
function panels_frame_stack_frame_delete_submit(&$form, &$form_state) {
  $cache_mechanism = $form_state['cache_mechanism'];
  $cache_key = $form_state['cache_key'];
  $name = $form_state['frame'];

  $object = $form_state['object'];
  unset($object->data[$name]);
  ctools_cache_set($cache_mechanism, $cache_key, $object);
}

/**
 * Validation handler to prevent nested layouts.
 * @fixme Need a proper helper that builds layout name (ex: panels_frame:stack:name).
 */
function panels_frame_stack_frame_layout_validate(&$form, &$form_state) {
  $name = $form_state['object']->name;
  $frame = $form_state['values']['identifier'];
  $layout = $form_state['values']['layout'];
  if ($layout == 'panels_frame_stack:' . $name) {
    form_set_error('layout', t('You cannot use %layout as a child layout as this will cause an infinite loop!', array('%layout' => $name)));
  }
}

/**
 * Common submission handler for add/edit/clone operations.
 */
function panels_frame_stack_frame_common_submit(&$form, &$form_state) {
  $cache_mechanism = $form_state['cache_mechanism'];
  $cache_key = $form_state['cache_key'];
  $object = $form_state['object'];

  foreach (array('label', 'identifier', 'weight', 'layout') as $key) {
    $frame[$key] = $form_state['values'][$key];
  }

  $object->data[$frame['identifier']] = $frame;
  ctools_cache_set($cache_mechanism, $cache_key, $object);
}

/**
 * Element validate callback to sort frames.
 */
function panels_frame_stack_ui_frames_sort(&$element, &$form_state) {
  $value = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  if ($value) {
    uasort($value, 'drupal_sort_weight');
    drupal_array_set_nested_value($form_state['values'], $element['#parents'], $value);
  }
}

/**
 * After build callback for frame data table.
 */
function panels_frame_stack_ui_frames_after_build(&$element, &$form_state) {
  $id = 'panels-frame-stack-ui-table';
  ctools_add_css('panels-frame.ui', 'panels_frame');
  ctools_add_css('panels-frame.ui-stack', 'panels_frame');
  drupal_add_tabledrag($id, 'order', 'sibling', 'panels-frame-stack-frame-weight');

  // Explicitly naming our headers for form elements which DO have a UI.
  $table['header'] = array(
    array('data' => t('Icon'), 'class' => 'icon'),
    array('data' => t('Title'), 'class' => 'title'),
    array('data' => t('Weight'), 'class' => 'weight'),
    array('data' => t('Operations'), 'class' => 'operations'),
  );

  $table['rows'] = array();
  // Loop through each frame.
  foreach (element_children($element) as $frame) {
    $rows = array();
    // Loop through each property.
    foreach (element_children($element[$frame]) as $property) {
      // Filter out the properties that have no UI.
      if (!in_array($property, $form_state['no_ui'])) {
        $rows[] = array('data' => render($element[$frame][$property]), 'class' => $property);
      }
    }
    $table['rows'][] = array(
      'class' => array('draggable'),
      'data' => $rows,
    );
  }

  $table['attributes']['id'] = $id;
  $table['attributes']['class'][] = 'panels-frame-ui';
  $table['attributes']['class'][] = $id;
  $table['sticky'] = TRUE;
  $table['empty'] = t('No frames to display.');

  $element['#children'] = theme('table', $table);

  return $element;
}
