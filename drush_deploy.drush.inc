<?php

/**
 * @file
 * drush_deploy keeps an instance of a Drupal project up to date 
 */

/**
 * Implementation of hook_drush_help().
 */
function drush_deploy_drush_help($section) {
  switch ($section) {
    case 'drush:drush_deploy':
      return dt('Update an instance of a Drupal project via drush.');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function drush_deploy_drush_command() {
  $items = array();

  $items['drush-deploy'] = array(
    'description' => 'Update an instance of a Drupal project via drush',
    'aliases' => array('deploy'),
  );

  return $items;
}

/**
 * Command callback for drush deploy.
 */
function drush_drush_deploy() {
  $commands = (drush_get_option('deploy-commands', drush_deploy_default_commands()));
  foreach ($commands as $i => $command) {
    //Determine drush command and arguments
    $args = array(); //defaults to no args
    if (is_array($command)) {
    	//FIXME: Is there a simpler way of extracting first key-value pair from an array?
    	foreach ($command as $command => $args) {
    		break;
    	}
    }
    
    //Execute command
    $success = call_user_func_array('drush_invoke', array_merge(array($command), $args));

    //Exit in case of emergency!
    if (!$success) {
    	//TODO: Consider if rollback of already executed commands is possible
    	break;
    }
  }
}

/**
 * Assemble a set of sensible default deploy commands when using features and git.
 */
function drush_deploy_default_commands() {
  $default_commands = array();
  
  //Set site offline
  $default_commands[] = array('variable_set' => array('site_offline', 1));
  
  //If git branch or tag has been defined we probably want to use
  //gittyup for updating the source code
  if (drush_get_option('git-branch') || 
      drush_get_option('git-tag')) {
    $default_commands[] = 'gittyup';
  }
  
  //Perform database updates
  $default_commands[] = 'updatedb';
  
  //Only revert features if used
  if (function_exists(drush_features_revert_all)) {
    $default_commands[] = 'features_revert_all';
    $default_commands[] = 'features_list';
  }
  
  //Make sure we have a clean cache
  $default_commands[] = array('cache_clear' => array('all'));
  
  // TODO: Hit the frontpage as admin user while site is offline 
  // to warm caches and avoid stampede.
  
  //Set site online again
  $default_commands[] = array('variable_set' => array('site_offline', 0));

  return $default_commands;
}