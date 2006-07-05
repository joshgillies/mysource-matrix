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
* $Id: asset_edit_fns.inc,v 1.57.2.1 2006/07/05 03:29:38 rong Exp $
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
* @version $Revision: 1.57.2.1 $
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
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'menu',
												   ),
							'permissions'		=> Array(
													'name'			=> 'Permissions',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'permissions',
												   ),
							'workflow'			=> Array(
													'name'			=> 'Workflow',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'workflow',
												   ),
							'roles'				=> Array(
													'name'			=> 'Roles',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'roles',
												   ),
							'metadataSchemas'	=> Array(
													'name'			=> 'Metadata Schemas',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'metadata',
												   ),
							'metadata'			=> Array(
													'name'			=> 'Metadata',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'metadata',
												   ),
							'dependants'		=> Array(
													'name'			=> 'Dependants',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'linking'			=> Array(
													'name'			=> 'Linking',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'links',
												   ),
							'settings'			=> Array(
													'name'			=> 'Settings',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'all',
												   ),
							'history'			=> Array(
													'name'			=> 'History',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'logs'				=> Array(
													'name'			=> 'Logs',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'preview'			=> Array(
													'name'			=> 'Preview',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'lookupValues'		=> Array(
													'name'			=> 'Lookup Settings',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'lookupValues',
												   ),
							'tagging'			=> Array(
													'name'			=> 'Tagging',
													'force_unlock'	=> TRUE,
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaths(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_paths'])) return FALSE;
		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']))) {
			return FALSE;
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);

		return TRUE;

	}//end processPaths()


	/**
	* Paint the interface for the asset paths
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
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
	* @param int	&$asset	the asset to check workflow completness for
	*
	* @return boolean
	* @access public
	*/
	function isWorkflowComplete(&$asset)
	{
		return $GLOBALS['SQ_SYSTEM']->wm->isWorkflowComplete($asset->id);

	}//end isWorkflowComplete()


	/**
	* Paint a generic interface for editing assets that are notice linked to this asset
	*
	* Returns true if editable interface printed
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param array	$link_value	the current link
	* @param array	$type_codes	type_codes to pass to asset_finder()
	* @param string	$field_name	field_name to pass to asset_finder()
	*
	* @return boolean
	* @access public
	*/
	function paintNoticeLinkedAsset(&$asset, $link_value, $type_codes=Array(), $field_name='')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);
		$selected = (empty($link)) ? 0 : $link['minorid'];

		if (empty($field_name)) {
			$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));
		}

		if (!$asset->writeAccess('links')) {
			echo '<i>';
			if (!$selected) {
				echo 'None';
			} else {
				echo get_asset_tag_line($selected);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($linked_asset);
			}
			echo '</i>';
			return FALSE;
		}

		asset_finder($field_name, $selected, $type_codes);

		return TRUE;

	}//end paintNoticeLinkedAsset()


	/**
	* Process a generic interface for editing assets that are notice linked to this asset
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param string	$link_value	the value to put on the link
	* @param string	$field_name	the name used for the asset finder
	*
	* @return boolean
	* @access public
	*/
	function processNoticeLinkedAsset(&$asset, $link_value, $field_name='')
	{
		if (empty($field_name)) {
			$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));
		}
		if (!isset($_POST[$field_name])) return FALSE;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);

		// if there has been no change, then there is nothing to do
		if (!empty($link) && $link['minorid'] == $_POST[$field_name]['assetid']) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$success = TRUE;

		// if there is a current link, we need to delete it
		if (!empty($link)) {
			$success = (bool) $asset->deleteLink($link['linkid']);
		}

		if ($success && !empty($_POST[$field_name]['assetid'])) {
			$selected_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$field_name]['assetid']);
			if (!is_null($selected_asset)) {
				$success = (bool) $asset->createLink($selected_asset, SQ_LINK_NOTICE, $link_value);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $success;

	}//end processNoticeLinkedAsset()


	/**
	* Return true if the search manager asset type is installed
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isSearchInstalled(&$asset, $prefix)
	{
		return $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');

	}//end isSearchInstalled()


}//end class

?>
