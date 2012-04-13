<?php

class panels_frame_stack_ui extends ctools_export_ui {

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
  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);

    $options = array('all' => t('- All -'));
    foreach ($this->items as $item) {
      $options[$item->category] = $item->category;
    }

    $form['top row']['category'] = array(
      '#type' => 'select',
      '#title' => t('Category'),
      '#options' => $options,
      '#default_value' => 'all',
      '#weight' => -10,
    );
  }

  function list_filter($form_state, $item) {
    if ($form_state['values']['category'] != 'all' && $form_state['values']['category'] != $item->category) {
      return TRUE;
    }

    return parent::list_filter($form_state, $item);
  }

  function edit_form(&$form, &$form_state) {
    // $form['helior'] = panels_frame_choose_layout($form, $form_state);

    parent::edit_form($form, $form_state);

    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    ctools_add_css('panels_dnd', 'panels');

    $form['info']['label']['#title'] = t('Label');
    $form['info']['label']['#size'] = 30;
    $form['info']['name']['#size'] = 30;

    $form['info']['description']['#rows'] = 2;
    $form['info']['description']['#resizable'] = FALSE;

    $form['info']['category'] = array(
      '#type' => 'textfield',
      '#size' => 24,
      '#default_value' => $form_state['item']->category,
      '#title' => t('Category'),
      '#description' => t('What category this layout should appear in. If left blank the category will be "Miscellaneous".'),
    );

    $form['frames'] = array('#type' => 'fieldset');
    $form['frames']['data'] = array(
      '#after_build' => array('panels_frame_stack_ui_frames_after_build'),
      '#tree' => TRUE,
    );

    $fake = $this->fake_elements();
    foreach ($form_state['item']->data as $name => $frame) {
      // Preview
      $form['frames']['data'][$name]['preview'] = array(
        '#markup' => $fake[$name]['preview'],
      );

      // Title
      $form['frames']['data'][$name]['title'] = array(
        '#markup' => $fake[$name]['layout'] . '(' . $name . ')',
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
      '#value' => url('panels_frame/ajax/stack/frame/add', array('absolute' => TRUE)),
    );
  }

  function edit_form_validate(&$form, &$form_state) {
    parent::edit_form_validate($form, $form_state);

    uasort($form_state['values']['data'], 'drupal_sort_weight');
  }

  function fake_elements() {
    return array(
      'first' => array(
        'layout' => 'Three Column Stacked',
        'preview' => '(☞ﾟヮﾟ)☞',
      ),
      'second' => array(
        'layout' => 'One Column',
        'preview' => '(•ω•)',
      ),
      'third' => array(
        'layout' => 'Two Column Hipster',
        'preview' => 'ヽ(´ｰ｀ )ﾉ',
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
