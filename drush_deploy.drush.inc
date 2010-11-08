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
    'description' => "Update an instance of a Drupal project via drush",
    'callback arguments' => array(array(), FALSE),
    'options' => array(
      '--deploy-host' => 'The hostname of the site to deploy to',
    ),
    'aliases' => array('deploy'),
  );

  return $items;
}

/**
 * Command callback for drush deploy.
 */
function drush_drush_deploy() {
	// Set site offline
  drush_variable_set('site_offline', 1);
  	
	// Update source code
  if (drush_get_option('git-branch') || 
      drush_get_option('git-tag') || 
      (drush_get_option('deploy-method') == 'git')) {
		drush_gittyup();
  }
  
	// Update database
	drush_core_updatedb();
  
	// Revert features
	if (function_exists('drush_features_revert_all')) {
  	drush_features_revert_all();
  	
  	// Detect and list features still in non-default state
  	// @see drush_features_list()
  	module_load_include('inc', 'features', 'features.export');
  	$rows = array();
	  foreach (features_get_features(NULL, TRUE) as $k => $m) {
	    switch (features_get_storage($m->name)) {
	      case FEATURES_DEFAULT:
	      case FEATURES_REBUILDABLE:
	        $storage = '';
	        break;
	      case FEATURES_OVERRIDDEN:
	        $storage = dt('Overridden');
	        break;
	      case FEATURES_NEEDS_REVIEW:
	        $storage = dt('Needs review');
	        break;
	    }
	    if ($storage != '') {
		    $rows[] = array(
		      $m->info['name'],
		      $m->name,
		      $m->status ? dt('Enabled') : dt('Disabled'),
		      $storage
		    );
	    }
	  }

	  if (sizeof($rows) > 0) {
	  	drush_log(dt('Unable to revert the following features to default state:'), 'warning');

	  	array_unshift($rows, array(array(dt('Name'), dt('Feature'), dt('Status'), dt('State'))));
      drush_print_table($rows, TRUE);
      
      // Display features diff for overridden modules if available
      if (module_exists('diff')) {
      	foreach ($rows as $row) {
      		drush_log(dt('Diff for module %module', array('%module' => $row[0])));
      		drush_features_diff($row[0]);
      	}
      }
	  }
	}
	  	
	// Clear cache
	drupal_flush_all_caches();
	
	// TODO: Hit the frontpage while site is offline to warm caches and avoid stampede
	
	// Set site online
	drush_variable_set('site_offline', 0);
}