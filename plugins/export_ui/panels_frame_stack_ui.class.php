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
    $fake_form_state = array(
      'cache_mechanism' => $cache_mechanism,
      'cache_key' => $cache_key,
    );

    $form['data'] = panels_frame_stack_ui_frames_table($form_state['item']->data, $fake_form_state);

    $form['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => 'panels-frame-stack-frame-add',
      '#value' => t('Moar!'),
    );

    $form['add-url'] = array(
      '#attributes' => array('class' => array("panels-frame-stack-frame-add-url")),
      '#type' => 'hidden',
      '#value' => url("panels_frame/stack/frame/ajax/add/$cache_mechanism/$cache_key", array('absolute' => TRUE)),
    );
  }
}
