<?php

class panels_frame_stack_ui extends panels_frame_ui {

  function hook_menu(&$items) {
    $base = array(
      'access arguments' => array('access content'),
      'type' => MENU_CALLBACK,
      'file' => 'panels_frame.admin.inc',
      'file path' => drupal_get_path('module', 'panels_frame'),
      'theme callback' => 'ajax_base_page_theme',
    );

    $items['panels_frame/ajax/stack/frame/add'] = array(
      'page callback' => 'panels_frame_ajax_stack_frame_add',
    ) + $base;

    parent::hook_menu($items);
  }

  function edit_form_frames(&$form, &$form_state) {
    ctools_include('plugins', 'panels');
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    ctools_add_css('panels_dnd', 'panels');

    $cache_mechanism = 'export_ui::' . $form_state['plugin']['name'];
    $cache_key = $form_state['object']->edit_cache_get_key($form_state['item'], $form_state['form type']);

    $form['data'] = array(
      '#element_validate' => array('panels_frame_stack_ui_frames_sort'),
      '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
      '#tree' => TRUE,
    );

    foreach ($form_state['item']->data as $name => $frame) {
      foreach (array('label', 'identifier', 'layout') as $hidden_value) {
        $form['data'][$name][$hidden_value] = array(
          '#type' => 'value',
          '#value' => $frame[$hidden_value],
        );
      }

      $layout = panels_get_layout($frame['layout']);
      // Preview
      $form['data'][$name]['preview'] = array(
        '#markup' => panels_print_layout_icon($layout['name'], $layout),
      );

      // Display Title
      $form['data'][$name]['title']['#markup'] = implode('<br />', array(
        '<strong>' . $frame['label'] . '</strong>',
        '<em>' . $layout['title'] . '</em>',
      ));

      // Weight
      $form['data'][$name]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $frame['weight'],
        '#attributes' => array('class' => array('panels-frame-stack-frame-weight')),
      );

      // Operations
      $form['data'][$name]['operations'] = array(
        '#type' => 'link',
        '#title' => t('Configure'),
        '#href' => 'admin/structure',
      );
    }

    $form['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => 'panels-frame-stack-frame-add',
      '#value' => t('Moar!'),
    );

    $form['add-url'] = array(
      '#attributes' => array('class' => array("panels-frame-stack-frame-add-url")),
      '#type' => 'hidden',
      '#value' => url('panels_frame/ajax/stack/frame/add/' . $cache_mechanism . '/' . $cache_key, array('absolute' => TRUE)),
    );
  }
}

/**
 * After build callback for frame data table.
 */
function panels_frame_stack_ui_frames_after_build(&$element, &$form_state) {
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

  $element['#markup'] = theme('table', $table);
  return $element;
}
