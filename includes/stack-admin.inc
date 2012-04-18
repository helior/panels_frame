<?php

/**
 * Ajax menu callback to add a fram to a Panels Frame Stack.
 * @todo Make this callback degradable to nojs :\
 */
function panels_frame_stack_frame_ajax_delegate($op, $cache_mechanism, $cache_key, $name = NULL) {
  ctools_include('ajax');
  ctools_include('cache');
  ctools_include('modal');

  if (empty($op) || empty($cache_mechanism) || empty($cache_key)) {
    ctools_ajax_render_error();
  }

  // Load stored object from cache.
  if (!($object = ctools_cache_get($cache_mechanism, $cache_key))) {
    ctools_ajax_render_error(t('Invalid object name.'));
  }

  $form_state = array(
    'title' => t('!op Frame @name', array('!op' => ucwords($op), '@name' => $name)),
    'ajax' => TRUE,
    'cache_mechanism' => $cache_mechanism,
    'cache_key' => $cache_key,
    'frame' => $name,
    'object' => $object,
    'op' => $op,
  );

  $commands = ctools_modal_form_wrapper("panels_frame_stack_frame_$op", $form_state);

  if ($form_state['executed']) {
    $commands = array();
    // @todo render the table-drag correctly and replace on page.
    // @fixme Possibly have to render the entire edit form again.
    // $table = panels_frame_stack_ui_frames_table($object->data);
    // $commands[] = ajax_command_replace('table#panels-frame-stack-ui-table', render($table));
    $commands[] = ctools_modal_command_dismiss();
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
    '#value' => t('Add'),
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
    '#submit' => array('panels_frame_stack_frame_common_submit'),
  );

  return $form;
}

/**
 * Form builder for "delete"-ing a frame.
 * @todo Roll your own confirm_form which will allow a custom "cancel" link
 * that uses ajax commands to close the modal dialog.
 */
function panels_frame_stack_frame_delete($form, &$form_state) {
  $question = t('Are you sure you want to remove the frame @name?', array('@name' => $form_state['frame']));
  $form = confirm_form($form, $question);
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
 * Common submision handler for add/edit/clone operations.
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
 * Form element builder for frames table.
 * @todo this only renders properly in the context of a form build, not just element render.
 */
function panels_frame_stack_ui_frames_table($data, &$form_state) {
  $cache_mechanism = $form_state['cache_mechanism'];
  $cache_key = $form_state['cache_key'];

  $element = array(
    '#element_validate' => array('panels_frame_stack_ui_frames_sort'),
    '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
    '#tree' => TRUE,
  );

  foreach ($data as $name => $frame) {
    foreach (array('label', 'identifier', 'layout') as $hidden_value) {
      $element[$name][$hidden_value] = array(
        '#type' => 'value',
        '#value' => $frame[$hidden_value],
      );
    }

    // Preview
    $layout = panels_get_layout($frame['layout']);
    $element[$name]['preview'] = array(
      '#markup' => panels_print_layout_icon($layout['name'], $layout),
    );

    // Display Title
    $element[$name]['title']['#markup'] = implode('<br />', array(
      '<strong>' . $frame['label'] . '</strong>',
      '<em>' . $layout['title'] . '</em>',
    ));

    // Weight
    $element[$name]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $frame['weight'],
      '#attributes' => array('class' => array('panels-frame-stack-frame-weight')),
    );

    // Operations
    $operations = array(
      array(
        'title' => t('Edit'),
        'href' => "panels_frame/stack/frame/ajax/edit/$cache_mechanism/$cache_key/$name",
        'attributes' => array('class' => array('use-ajax')),
      ),
      array(
        'title' => t('Clone'),
        'href' => "panels_frame/stack/frame/ajax/clone/$cache_mechanism/$cache_key/$name",
        'attributes' => array('class' => array('use-ajax')),
      ),
      array(
        'title' => t('Delete'),
        'href' => "panels_frame/stack/frame/ajax/delete/$cache_mechanism/$cache_key/$name",
        'attributes' => array('class' => array('use-ajax')),
      ),
    );

    $element[$name]['operations'] = array(
      '#theme' => 'links__ctools_dropbutton',
      '#links' => $operations,
      '#attributes' => array('class' => array('links', 'inline')),
    );
  }

  return $element;
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
function panels_frame_stack_ui_frames_after_build(&$element) {
  $id = 'panels-frame-stack-ui-table';
  drupal_add_tabledrag($id, 'order', 'sibling', 'panels-frame-stack-frame-weight');

  $table['header'] = array(
    array('data' => t('Preview'), 'class' => 'previeww'),
    array('data' => t('Title'), 'class' => 'titlee'),
    array('data' => t('Weight'), 'class' => 'weightt'),
    array('data' => t('Operations'), 'class' => 'operationss'),
  );

  $table['rows'] = array();
  foreach (element_children($element) as $child) {
    $table['rows'][] = array(
      'class' => array('draggable'),
      'data' => array(
        render($element[$child]['preview']),
        render($element[$child]['title']),
        render($element[$child]['weight']),
        render($element[$child]['operations'])
      ),
    );
  }

  $table['attributes']['id'] = $id;
  $table['sticky'] = TRUE;
  $table['empty'] = t('Why you no have layout?');

  $element['#children'] = theme('table', $table);

  return $element;
}
