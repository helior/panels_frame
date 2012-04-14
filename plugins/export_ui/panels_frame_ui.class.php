<?php

class panels_frame_ui extends ctools_export_ui {

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
    parent::edit_form($form, $form_state);

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

  }
}
