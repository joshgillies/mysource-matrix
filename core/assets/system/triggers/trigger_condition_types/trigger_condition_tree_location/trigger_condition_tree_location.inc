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
* $Id: trigger_condition_tree_location.inc,v 1.3 2005/02/23 05:12:06 gsherwood Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Condition Tree Location
*
* Checks whether an asset is under a particular branch of a tree
*
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Tree_Location extends Trigger_Condition
{

	/**
	* Evaluate this condition
	*
	* @param array(string=>mixed)	$settings	the stored settings for this condition
	* @param array(string=>mixed)	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		/* settings used by this condition:
		$settings = Array(
			'assetid'		=> int,		// either assetid or treeid needs to be set. the asset id of the parent asset
			'treeid'		=> string,	// can be retrieved from assetid if necessary. the raw tree id of the parent asset
			'immed_child'	=> int		// 1 or 0. determines whether the asset being checked is supposed to be an immediate child, or any child
		);
		*/

		if (!empty($state['assetid'])) {
			$assetid = $state['assetid'];
		} else {
			if (!empty($state['asset'])) {
				$assetid = $state['asset']->id;
			} else {
				// can't retrieve, fail
				return false;
			}
		}

		if (empty($settings['treeid'])) {
			if (empty($settings['assetid'])) {
				return false;
			} else {
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
					$settings['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getRawTreeIdByAsset($settings['assetid'], true);
				} else {
					return false;
				}
			}
		}

		if (!isset($settings['immed_child'])) {
			return false;
		}

		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['treeid'])) {
			// not stored, need to retrieve
			$assetid = 0;
			$state['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getRawTreeIdByAsset($assetid, true);
		}

		if (!is_array($state['treeid'])) {
			$state['treeid'] = Array($state['treeid']);
		}


		// evaluate the condition
		$result = false;

		foreach ($state['treeid'] as $state_tree) {
			foreach ($settings['treeid'] as $settings_tree) {

				// if the asset in question is under the tree specified in the settings...
				if ($settings_tree == $state_tree) {
					$result = false;
					break;
				} else {
					if ($settings['immed_child']) {
						if (strlen($settings_tree) == strlen($state_tree) - SQ_CONF_ASSET_TREE_SIZE &&
						substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
							$result = true;
							break;
						}
					} else {
						if (substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
							$result = true;
							break;
						}
					}
				}//end if

			}
		}

		return $result;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param Array 		$settings  settings that this condition save in processInterface()
	* @param string 	$prefix    unique prefix
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix)
	{
		// set to default
		$selected_assetid = array_get_index($settings, 'assetid', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$immed_child = array_get_index($settings, 'immed_child', '0');
		$immed_child = ($immed_child == '0' ? false : true);

		// capture anything printed
		ob_start();

		// create an asset_finder, and pre-select the asset that is saved in settings (if any)
		?>
		Asset is under this tree node:
		<?php
		asset_finder($prefix.'[assetid]', $selected_assetid);
		check_box($prefix.'[immed_child]', '1', $immed_child);
		?>
		Immediate child only
		<?php

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param Array			$settings		a container for any data the condition might want to save
	* @param object Hash	$hash			hash object that takes data that helps to speed up trigger firing
	* @param array			$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		if (empty($request_data) || empty($request_data['assetid']['assetid'])) {
			return false;
		}

		if (empty($request_data['immed_child'])) {
			$immed_child = '0';
		} else {
			$immed_child = '1';
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		if (!$am->assetExists($request_data['assetid']['assetid'])) {
			return false;
		};

		$treeid = $am->getRawTreeIdByAsset($request_data['assetid']['assetid']);

		// request_data['assetid'] contains 'assetid' and 'url' - we only want the id
		$settings['assetid'] 		= $request_data['assetid']['assetid'];
		$settings['immed_child']	= $immed_child;

		// only use the raw tree value in the hash
		$hash->setLinkTree($treeid, SQ_TRIG_TREE_PROPAGATE);

		return true;

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* Default to false, children can override if necessary
	*
	* @access public
	* @return boolean
	*/
	function allowMultiple()
	{
		// default to to disallow
		return true;

	}//end allowMultiple()


}//end class

?>
