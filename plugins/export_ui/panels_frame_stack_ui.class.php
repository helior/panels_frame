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

    // $op/$cache_key/$name
    $items['panels_frame/stack/frame/ajax/%/%'] = array(
      'page callback' => 'panels_frame_stack_frame_ajax_delegate',
      'page arguments' => array(4, 5),
    ) + $base;

    parent::hook_menu($items);
  }

  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);

    ctools_include('stack-admin', 'panels_frame');
    ctools_include('plugins', 'panels');
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    ctools_add_css('panels_dnd', 'panels');
    ctools_add_css('panels-frame.ui-stack', 'panels_frame');

    $form['info']['#type'] = 'container';
    $form['info']['#attributes']['class'][] = 'stack-admin-info';

    $form['buttons']['#type'] = 'container';
    $form['buttons']['#attributes']['class'][] = 'stack-admin-buttons';

    $form['info']['plugin'] = array(
      '#type' => 'value',
      '#value' => 'stack',
    );

    // Set the cache identifier and immediately set an object cache.
    $form_state['cache_key'] = $cache_key = 'edit-' . $form_state['item']->name;
    if (is_object($cache = panels_frame_cache_get('stack', $cache_key))) {
      $item = $cache;
    } else {
      $item = $form_state['item'];
    }
    panels_frame_cache_set('stack', $cache_key, $item);

    // Call out the values that will have no UI here. It will be referenced in
    // multiple places.
    $form_state['no_ui'] = array('label', 'identifier', 'layout');

    $form['frames']['#type'] = 'container';
    $form['frames']['#attributes']['class'][] = 'stack-admin-frames';

    // Built as a table-drag interface later..
    $form['frames']['data'] = array(
      '#element_validate' => array('panels_frame_stack_ui_frames_sort'),
      '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
      '#tree' => TRUE,
    );

    foreach ($item->data as $frame_id => $frame) {
      foreach ($form_state['no_ui'] as $hidden) {
        $form['frames']['data'][$frame_id][$hidden] = array(
          '#type' => 'value',
          '#value' => $frame[$hidden],
        );
      }

      // Icon
      $layout = panels_get_layout($frame['layout']);
      $form['frames']['data'][$frame_id]['icon'] = array(
        '#markup' => panels_print_layout_icon($layout['name'], $layout),
      );

      // Display Title
      $form['frames']['data'][$frame_id]['title']['#markup'] = implode('<br />', array(
        '<strong>' . $frame['label'] . '</strong>',
        '<em>' . $layout['title'] . '</em>',
      ));

      // Weight
      $form['frames']['data'][$frame_id]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $frame['weight'],
        '#attributes' => array('class' => array('panels-frame-stack-frame-weight')),
      );

      // Operations
      $operations = array(
        array(
          'title' => t('Edit'),
          'href' => "panels_frame/stack/frame/ajax/edit/$cache_key/$frame_id",
          'attributes' => array('class' => array('use-ajax')),
        ),
        array(
          'title' => t('Clone'),
          'href' => "panels_frame/stack/frame/ajax/clone/$cache_key/$frame_id",
          'attributes' => array('class' => array('use-ajax')),
        ),
        array(
          'title' => t('Delete'),
          'href' => "panels_frame/stack/frame/ajax/delete/$cache_key/$frame_id",
          'attributes' => array('class' => array('use-ajax')),
        ),
      );

      $form['frames']['data'][$frame_id]['operations'] = array(
        '#theme' => 'links__ctools_dropbutton',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      );
    }

    $form['frames']['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => 'panels-frame-stack-frame-add',
      '#value' => t('Add frame'),
    );

    $form['frames']['add-url'] = array(
      '#attributes' => array('class' => array("panels-frame-stack-frame-add-url")),
      '#type' => 'hidden',
      '#value' => url("panels_frame/stack/frame/ajax/add/$cache_key", array('absolute' => TRUE)),
    );
  }

  function edit_form_submit(&$form, &$form_state) {
    parent::edit_form_submit($form, $form_state);

    // The frame should be saved to the database now, so we should be able to
    // remove the object cache.
    panels_frame_cache_clear('stack', $form_state['cache_key']);
  }
}
