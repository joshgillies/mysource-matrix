<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: asset_edit_fns.inc,v 1.49 2005/07/13 04:05:57 mmcintyre Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.49 $
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var array
	*/
	var $static_screens = Array(
							'details'			=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> true,
													'lock_type'		=> 'menu',
												   ),
							'permissions'		=> Array(
													'name'			=> 'Permissions',
													'force_unlock'	=> false,
													'lock_type'		=> 'permissions',
												   ),
							'workflow'			=> Array(
													'name'			=> 'Workflow',
													'force_unlock'	=> true,
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
													'lock_type'		=> 'none',
												   ),
							'linking'			=> Array(
													'name'			=> 'Linking',
													'force_unlock'	=> true,
													'lock_type'		=> 'links',
												   ),
							'settings'			=> Array(
													'name'			=> 'Settings',
													'force_unlock'	=> true,
													'lock_type'		=> 'all',
												   ),
							'history'			=> Array(
													'name'			=> 'History',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'logs'				=> Array(
													'name'			=> 'Logs',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'preview'			=> Array(
													'name'			=> 'Preview',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'lookupValues'		=> Array(
													'name'			=> 'Lookup Settings',
													'force_unlock'	=> false,
													'lock_type'		=> 'all',
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

		foreach ($paths as $path) {
			if ($wa) {
				text_box($prefix.'_paths[]', $path, 25);
			} else {
				echo $path;
			}
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

		return $wa;

	}//end paintPaths()


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
		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']))) {
			return false;
		}

		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);

		return true;

	}//end processPaths()


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
			echo translate('no_associated_urls');
		} else {
			usort($urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
			foreach ($urls as $data) {
				if ($data['http']) {
					echo 'http://'.$data['url'], '<br/>';
				}
				if ($data['https']) {
					echo 'https://'.$data['url'], '<br/>';
				}
			}
		}

		if ($asset->force_secure) {
			echo '<br>';
			$o->note(translate('force_secure_set_on_asset'));
		}

	}//end paintURLs()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int    $assetid        the ID of the asset to check workflow completness for
	*
	* @return boolean
	* @access public
	*/
	function isWorkflowComplete(&$asset)
	{
		return $GLOBALS['SQ_SYSTEM']->wm->isWorkflowComplete($asset->id);

	}//end isWorkflowComplete()


}//end class

?>
