<?php

function panels_frame_temp_admin($form, &$form_state) {
  $form['temp']['#markup'] = 'Hi. Admin form placeholder. Sorry.';

  return $form;
}