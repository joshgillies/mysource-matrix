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
* $Id: trigger_action_create_clone.inc,v 1.1 2005/02/17 06:34:05 rhoward Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Clone
*
* A trigger action that creates a clone of the firing asset
*
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Clone extends Trigger_Action
{

	/**
	* Execute this action
	*
	* @param array(string=>mixed)	$settings	the stored settings for this action
	* @param array(string=>mixed)	&$state	the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		/* settings used by this action:
		$settings = Array(
			'assetid'		=> int,		// the assetid to link to
			'link_type'		=> int,		// the type of link, eg. type_1, type_2, type_3 or notice
			'value'			=> string,	// optional, the value associated with a link
			'is_dependant'	=> int,		// optional, whether the link is dependant
			'is_exclusive'	=> int		// optional, whether the link is exclusive
		);
		*/

		// check settings, state
		if (empty($settings['assetid']) || empty($settings['link_type'])) {
			// if no settings, fail
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->isAsset($settings['assetid'])) {
			return false;
		}

		// check the optional settings, assign defaults
		$is_dependant		= array_get_index($settings, 'is_dependant', 0);
		$is_exclusive		= array_get_index($settings, 'is_exclusive', 0);

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$major = &$am->getAsset($settings['assetid']);
		$minor = &$state['asset'];


		// clone the asset

		// old -> new asset id mapping
		$clone_map = Array();

		// the asset components we want
		$components = Array(
			'all'
		);

		// the new link
		$link = Array(
			'asset'			=> &$major,
			'link_type'		=> $settings['link_type'],
			'value'			=> $settings['value'],
			'sort_order'	=> null,
			'is_dependant'	=> $is_dependant,
			'is_exclusive'	=> $is_exclusive
		);

		$clone = $am->cloneAsset(
			$minor,
			$link,
			$clone_map,
			$components
		);

		if (is_null($clone)) {
			error_log('null clone');
			return false;
		}

		$valid_paths = make_valid_web_paths($minor->getWebPaths());
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, true);

		$clone->saveWebPaths($good_paths);
		$clone->remapAssetids($clone_map);

		error_log('########### CLONE #############'.print_r($clone, true));
		error_log('######################################################################');
		error_log('########### ORIGINAL #############'.print_r($minor, true));

		error_log('clone created');

		// take the link and insert the relevant entries into the lookup tables, if required
//		$major->updateLookups();
//		$clone->updateLookups();

		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param Array(string=>mixed)	$settings  settings that this condition saves in processInterface()
	* @param string 				$prefix    unique prefix
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix)
	{
		// Create a clone of the asset, and link it as child of: [     asset] using a [Type 1] link.
		// <b>Optional items for Advanced users:</b>
		// Use this value as the link value: [Value]
		// Link the child [Non-dependantly] and [Non-exclusively] to the parent.


		// check settings, set defaults if necessary
		$selected_assetid 	= array_get_index($settings, 'assetid', 0);
		$selected_link_type	= array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);
		$value				= array_get_index($settings, 'value', '');
		$is_dependant		= array_get_index($settings, 'is_dependant', 0);
		$is_exclusive		= array_get_index($settings, 'is_exclusive', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->isAsset($selected_assetid)) {
			$selected_assetid = 0;
		}

		$link_type_list	= get_link_type_names();
		unset($link_type_list[SQ_LINK_NOTICE]);


		// capture anything printed
		ob_start();

		?>
		Create a clone of the asset, and link it as child of:
		<?php
		asset_finder($prefix.'[assetid]', $selected_assetid);
		?>
		using a
		<?php
		combo_box($prefix.'[link_type]', $link_type_list, false, $selected_link_type);
		?>
		link.<br /><br />

		<b>Optional items for Advanced users:</b><br />
		Use this value as the link value: <?php text_box($prefix.'[value]', $value); ?> and

		link the child
		<?php
		$is_dependant_list = Array(
			0	=> 'Non-dependantly',
			1	=> 'Dependantly'
		);
		combo_box($prefix.'[is_dependant]', $is_dependant_list, false, $is_dependant);
		?>
		and
		<?php
		$is_exclusive_list = Array(
			0	=> 'Non-exclusively',
			1	=> 'Exclusively'
		);
		combo_box($prefix.'[is_exclusive]', $is_exclusive_list, false, $is_exclusive);
		?>
		to the parent.
		<?php


		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param Array(string=>mixed)	$settings		a container for any data the action might want to save
	* @param string					$request_data	array of data corresponding to this action as specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		$link_type_list	= get_link_type_names();

		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return false;
		}

		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return false;
		}

		$settings['assetid'] 		= $request_data['assetid']['assetid'];
		$settings['link_type']		= $request_data['link_type'];

		// optional fields
		$settings['value']			= array_get_index($request_data, 'value', '');
		$settings['is_dependant']	= array_get_index($request_data, 'is_dependant', 0);
		$settings['is_exclusive']	= array_get_index($request_data, 'is_exclusive', 0);

		return true;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing.
	*
	* @access public
	* @return array(assetid=>locktype)
	*/
	function getLocks($settings, &$state)
	{
		// get 'all' locks on the firing asset
		$locks = Array(
			$state['assetid']	=> Array(
				'all'
			)
		);

		$dependant_children = &$GLOBALS['SQ_SYSTEM']->am->getDependantChildren($state['assetid']);
		foreach ($dependant_children as $assetid => $asset_type) {
			$locks[$assetid][] = 'all';
		}

		return $locks;

	}//end getLocks()


}//end class

?>
