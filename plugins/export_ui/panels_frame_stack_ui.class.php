<?php

class panels_frame_stack_ui extends panels_frame_ui {

  function hook_menu(&$items) {
    $base = array(
      'access arguments' => array('access content'),
      'type' => MENU_CALLBACK,
      'file' => 'stack-admin.inc',
      'file path' => drupal_get_path('module', 'panels_frame') . '/includes',
      'theme callback' => 'ajax_base_page_theme',
    );

    // $op/$cache_mechanism/$cache_key/$name
    $items['panels_frame/stack/frame/ajax/%/%/%'] = array(
      'page callback' => 'panels_frame_stack_frame_ajax_delegate',
      'page arguments' => array(4,5,6),
    ) + $base;

    parent::hook_menu($items);
  }

  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);

    $form['info']['plugin'] = array(
      '#type' => 'value',
      '#value' => 'stack',
    );
  }

  function edit_form_frames(&$form, &$form_state) {
    ctools_include('stack-admin', 'panels_frame');
    ctools_include('plugins', 'panels');
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    ctools_add_css('panels_dnd', 'panels');

    $cache_mechanism = 'export_ui::' . $form_state['plugin']['name'];
    $cache_key = $form_state['object']->edit_cache_get_key($form_state['item'], $form_state['form type']);

    // Call out the values that will have no UI here. It will be referenced in
    // multiple places.
    $form_state['no_ui'] = array('label', 'identifier', 'layout');

    $form['data'] = array(
      '#element_validate' => array('panels_frame_stack_ui_frames_sort'),
      '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
      '#tree' => TRUE,
    );

    foreach ($form_state['item']->data as $name => $frame) {
      foreach ($form_state['no_ui'] as $hidden) {
        $form['data'][$name][$hidden] = array(
          '#type' => 'value',
          '#value' => $frame[$hidden],
        );
      }

      // Preview
      $layout = panels_get_layout($frame['layout']);
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

      $form['data'][$name]['operations'] = array(
        '#theme' => 'links__ctools_dropbutton',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      );
    }

    $form['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => 'panels-frame-stack-frame-add',
      '#value' => t('Add frame'),
    );

    $form['add-url'] = array(
      '#attributes' => array('class' => array("panels-frame-stack-frame-add-url")),
      '#type' => 'hidden',
      '#value' => url("panels_frame/stack/frame/ajax/add/$cache_mechanism/$cache_key", array('absolute' => TRUE)),
    );
  }
}
