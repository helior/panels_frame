<?php

$plugin = array(
  'schema' => 'panels_frame',
  'handler' => 'panels_frame_stack_ui',
  'access' => 'administer panels frame',

  'menu' => array(
    'menu prefix' => 'admin/structure/panels/frame',
    'menu item' => 'stacks',
    'menu title' => 'Stacks',
    'menu description' => 'Add, edit or delete stacks, single-dimension compilations of panels layouts.',
  ),

  'title singular' => t('stack'),
  'title singular proper' => t('Stack'),
  'title plural' => t('stacks'),
  'title plural proper' => t('Stacks'),
);