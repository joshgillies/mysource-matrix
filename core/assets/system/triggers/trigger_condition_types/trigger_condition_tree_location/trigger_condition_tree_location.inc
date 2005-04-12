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
* $Id: trigger_condition_tree_location.inc,v 1.6 2005/04/12 07:01:36 arailean Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Condition Tree Location
*
* Checks whether an asset is under a particular branch of a tree
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Tree_Location extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'assetid'       => int,     // either assetid or treeid needs to be set. the asset id of the parent asset
	*       'treeid'        => string,  // can be retrieved from assetid if necessary. the raw tree id of the parent asset
	*       'immed_child'   => int      // 1 or 0. determines whether the asset being checked is supposed to be an immediate child, or any child
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this condition
	* @param array(string=>mixed)   &$state     the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
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
					$settings['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($settings['assetid']);
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
			$state['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($assetid);
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
	* @param Array      $settings  settings that this condition save in processInterface()
	* @param string     $prefix    unique prefix
	* @param boolean    $write_access   write access flag
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// set to default
		$selected_assetid = array_get_index($settings, 'assetid', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$immed_child = array_get_index($settings, 'immed_child', '0');
		$immed_child = ($immed_child == '0' ? false : true);

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}


		// capture anything printed
		ob_start();

		// create an asset_finder, and pre-select the asset that is saved in settings (if any)
		echo "Asset is under this tree node:";
		if ($write_access) {
			asset_finder($prefix.'[assetid]', $selected_assetid);
		} else {
			if ($selected_assetid) {
				$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), true, 'name');
				text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', false, $form_element_extras);
			} else {
				text_box($prefix.'[assetid]', '', '', '', false, $form_element_extras);
			}
		}
		check_box($prefix.'[immed_child]', '1', $immed_child, null, $form_element_extras);
		echo "Immediate child only";

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param Array          $settings       a container for any data the condition might want to save
	* @param object Hash    $hash           hash object that takes data that helps to speed up trigger firing
	* @param array          $request_data   array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return false|string
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		if (empty($request_data) || empty($request_data['assetid']['assetid'])) {
			return 'No tree location selected';
		} else {
			$assetid = $request_data['assetid']['assetid'];
		}

		if (empty($request_data['immed_child'])) {
			$immed_child = '0';
			$propagate = SQ_TRIG_TREE_PROPAGATE;
		} else {
			$immed_child = '1';
			$propagate = SQ_TRIG_TREE_NO_PROPAGATE;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		if (!$am->assetExists($assetid)) {
			return 'Selected asset does not exist';
		};

		$treeid = $am->getAssetTreeids($assetid);

		// request_data['assetid'] contains 'assetid' and 'url' - we only want the id
		$settings['assetid']     = $assetid;
		$settings['immed_child'] = $immed_child;

		// only use the raw tree value in the hash
		$hash->setLinkTree($treeid, $propagate);

		return false;

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
