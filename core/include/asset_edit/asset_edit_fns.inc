<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_edit_fns.inc,v 1.23 2003/10/10 06:49:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $static_screens = Array('details'			=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> true,
														'lock_type'		=> 'attributes',
													),
								'permissions'		=> Array(
														'name'			=> 'Permissions',
														'force_unlock'	=> true,
														'lock_type'		=> 'permissions',
													),
								'workflow'			=> Array(
														'name'			=> 'Workflow',
														'force_unlock'	=> false,
														'lock_type'		=> 'workflow',
													),
								'metadataSchemas'	=> Array(
														'name'			=> 'Metadata Schemas',
														'force_unlock'	=> true,
														'lock_type'		=> 'metadata',
													),
								'metadata'			=> Array(
														'name'			=> 'Metadata',
														'force_unlock'	=> true,
														'lock_type'		=> 'metadata',
													),
								'dependants'		=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> true,
														'lock_type'		=> '',
													),
								'linking'			=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> true,
														'lock_type'		=> 'links',
													),
								'settings'			=> Array(
														'name'			=> 'Settings',
														'force_unlock'	=> false,
														'lock_type'		=> 'all',
													),
								'history'			=> Array(
														'name'			=> 'History',
														'force_unlock'	=> true,
														'lock_type'		=> '',
													),
								'logs'				=> Array(
														'name'			=> 'Logs',
														'force_unlock'	=> true,
														'lock_type'		=> '',
													),
								);


	/**
	* Constructor
	*
	*/
	function Asset_Edit_Fns()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Paint the interface for the asset paths
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPaths(&$asset, &$o, $prefix)
	{

		$wa = $asset->writeAccess('lookups');

		$paths = $asset->getWebPaths();

		foreach($paths as $path) {
			if ($wa) text_box($prefix.'_paths[]', $path, 25);
			else echo $path;
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

	}// end paintPaths()


	/**
	* Processes the Paths posted from the paintPaths fn above
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaths(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_paths'])) return false;
		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']))) return false;
		
		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);
		////  END HIPO PROCESSING  ////

		return true;

	}// end processPaths()


	/**
	* Paint the interface for the asset paths
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix)
	{
		$urls = $asset->getLookups();
		if (empty($urls)) {
			echo 'There are no URLs associated with this Asset';
		} else {
			foreach($urls as $data) {
				if ($data['http'])  echo 'http://'.$data['url'], '<br/>';
				if ($data['https']) echo 'https://'.$data['url'], '<br/>';
			}
		}

		if ($asset->force_secure) {
			echo '<br>';
			$o->note('Force Secure (see Settings screen) is set on this asset, so if you think that there should other URLs registered you may need to check that the site(s) that this asset is has allowed the https protocol');
		}

	}// end paintURLs()


}//end class

?>