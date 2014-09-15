<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Design Customisation
*
* A trigger action that creates a design customisation of the firing asset
*
*
* @author   Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Design_Customisation extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'customisation_name'                => string,     // the name of the customisation
	*       );
	* </PRE>
	*
	* (All we need is just the $state['asset'], we clone a design from there, morpth it and get the links.
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{

		if (empty($state['asset'])) {
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// This clone will be morphed to customisation later, so it will NOT clone the parse file.
		$asset->_tmp['custom'] = TRUE;
		$clone = $GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions', 'roles'), FALSE);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$default_name = 'Customisation #'.$clone->id;
		$new_name = array_get_index($settings, 'customisation_name', $default_name);

		$keywords_list = retrieve_keywords_replacements($new_name);
		$keywords_list = array_unique($keywords_list);
		$replacements = Array();
		foreach ($keywords_list as $keyword) {
			$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
		}
		$new_name = replace_keywords($new_name, $replacements);

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}


		// Now let's morph the dupe into a design customisation
		// since 3.18, morph() returns the result asset instead of a boolean
		$customisation = $clone->morph('design_customisation');



		if (!$customisation) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and link it back to ourselves
		if (!$asset->createLink($customisation, SQ_LINK_TYPE_2, 'customisation', NULL, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!($customisation->setAttrValue('id_name', $new_name) && $customisation->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach ($da_links as $link) {
			$da = $am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			if (!$customisation->createLink($da, SQ_LINK_TYPE_3, $link['value'], NULL, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $customisation->id, TRUE);
		if (!$customisation->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$minor = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0162', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// link up all the css files from this design to the customisation
		$css_link_ids = Array();
		$css_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'design_css', FALSE);
		foreach ($css_links as $link) {
			$minor = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0161', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return	Array(
					'assetid'			=> $customisation->id,
					'original_design'	=> $asset->id,
				);

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// How to get a list of keyword if there is no asset to get it from?
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('design');
		$tmp_design = new Design();
		$keywords = $tmp_design->getAvailableKeywords();
		ob_start();
			text_box($prefix.'[customisation_name]', '%asset_name% - Customisation', 50);
			$text_box = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[keywords]', $keywords, FALSE, '',0, 'onchange="document.getElementById(\''.$prefix.'[customisation_name]\').value += \' %\' + this.value + \'%\';"');
			$combo_box = ob_get_contents();
		ob_end_clean();

		$contents =	translate('trigger_create_design_customisation_usage')
		.'<br /><br />'.translate('trigger_create_design_customisation_select_keyword').$combo_box
		.'<br /><br />'.
		translate('trigger_create_design_customisation_specify_name').$text_box.'<br />'
		;

		return $contents;

	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// make sure the assetid isn't blank
		if (empty($request_data['customisation_name'])) {
			return translate('trigger_create_design_customisation_name_missing');
		}

		$settings['customisation_name']   = $request_data['customisation_name'];

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return	Array (
					$state['assetid']	=> Array('all'),
				);

	}//end getLocks()


}//end class


?>
