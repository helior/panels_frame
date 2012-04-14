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

  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);
    ctools_include('plugins', 'panels');
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    ctools_add_css('panels_dnd', 'panels');

    $cache_mechanism = 'export_ui::' . $form_state['plugin']['name'];

    $form['frames'] = array('#type' => 'fieldset');
    $form['frames']['data'] = array(
      '#element_validate' => array('panels_frame_stack_ui_frames_sort'),
      '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
      '#tree' => TRUE,
    );

    $fake = $this->fake_elements();
    foreach ($form_state['item']->data as $name => $frame) {
      $layout = panels_get_layout($fake[$name]['layout']);
      // Preview
      $form['frames']['data'][$name]['preview'] = array(
        '#markup' => panels_print_layout_icon($layout['name'], $layout),
      );

      // Title
      $form['frames']['data'][$name]['title'] = array(
        '#markup' => $layout['title'] . '<br>(' . $name . ')',
      );

      // Weight
      $form['frames']['data'][$name]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $frame['weight'],
        '#attributes' => array('class' => array('panels-frame-stack-frame-weight')),
      );

      // Operations
      $form['frames']['data'][$name]['operations'] = array(
        '#type' => 'link',
        '#title' => t('Configure'),
        '#href' => 'admin/structure',
      );
    }

    $form['frames']['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => 'panels-frame-stack-frame-add',
      '#value' => t('Moar!'),
    );

    $form['frames']['add-url'] = array(
      '#attributes' => array('class' => array("panels-frame-stack-frame-add-url")),
      '#type' => 'hidden',
      '#value' => url('panels_frame/ajax/stack/frame/add/' . $cache_mechanism . '/' . $form_state['item']->name, array('absolute' => TRUE)),
    );
  }

  function fake_elements() {
    return array(
      'first' => array(
        'layout' => 'threecol_25_50_25_stacked',
      ),
      'second' => array(
        'layout' => 'twocol',
      ),
      'third' => array(
        'layout' => 'twocol_bricks',
      ),
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
