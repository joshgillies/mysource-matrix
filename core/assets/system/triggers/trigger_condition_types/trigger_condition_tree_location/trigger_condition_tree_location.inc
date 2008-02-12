<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_condition_tree_location.inc,v 1.17 2008/02/12 23:45:11 bshkara Exp $
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
* @version $Revision: 1.17 $
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
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		if (!empty($state['assetid'])) {
			$assetid = $state['assetid'];
		} else {
			if (!empty($state['asset'])) {
				$assetid = $state['asset']->id;
			} else {
				// can't retrieve, fail
				return FALSE;
			}
		}

		if (empty($settings['treeid'])) {
			if (empty($settings['assetid'])) {
				return FALSE;
			} else {
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
					$settings['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($settings['assetid']);
				} else {
					return FALSE;
				}
			}
		}

		if (!isset($settings['immed_child'])) return FALSE;

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
		$result = FALSE;

		foreach ($state['treeid'] as $state_tree) {
			foreach ($settings['treeid'] as $settings_tree) {

				// if the asset in question is under the tree specified in the settings...
				if ($settings_tree === $state_tree) {
					$result = FALSE;
					break;
				} else {
					if ($settings['immed_child']) {
						if (strlen($settings_tree) == strlen($state_tree) - SQ_CONF_ASSET_TREE_SIZE &&
						substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
							$result = TRUE;
							break;
						}
					} else {
						if (substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
							$result = TRUE;
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
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// set to default
		$selected_assetid = array_get_index($settings, 'assetid', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$immed_child = array_get_index($settings, 'immed_child', '0');
		$immed_child = ($immed_child == '0' ? FALSE : TRUE);


		// begin buffering basic options
		ob_start();
			// create an asset_finder, and pre-select the asset that is saved in settings (if any)
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
				check_box($prefix.'[immed_child]', '1', $immed_child);
				label(translate('immediate_child_only'), $prefix.'[immed_child]');
			} else {
				if ($selected_assetid) {
					echo get_asset_tag_line($selected_assetid);
					if ($immed_child) {
						echo '<b> '.translate('as_an_immediate_child_only').'</b>';
					}
				} else {
					echo '<b>['.translate('no_asset_selected').']</b>';
				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return translate('trigger_tree_location_basic', $basic_part_1);

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data) || empty($request_data['assetid']['assetid'])) {
			return translate('triger_no_tree_location');
		} else {
			$assetid = $request_data['assetid']['assetid'];
		}

		if (empty($request_data['immed_child'])) {
			$immed_child = '0';
			$settings['propagate'] = SQ_TRIG_TREE_PROPAGATE;
		} else {
			$immed_child = '1';
			$settings['propagate'] = SQ_TRIG_TREE_NO_PROPAGATE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if (!$am->assetExists($assetid)) {
			return translate('selected_asset_does_not_exist');
		}

		$treeid = $am->getAssetTreeids($assetid);

		// request_data['assetid'] contains 'assetid' and 'url' - we only want the id
		$settings['assetid']     = $assetid;
		$settings['immed_child'] = $immed_child;

		return FALSE;

	}//end processInterface()


	/**
	* Sets the hash using the settings from the condition
	* Consult the hash object to see the list of setter functions
	*
	* @param array	&$settings	a container for any data the condition might want to save
	* @param object	&$hash		hash object that takes data that helps to speed up trigger firing
	*
	* @return mixed string|boolean
	* @access public
	*/
	public static function setHash(&$settings, &$hash)
	{
		if (isset($settings['assetid']) && isset($settings['propagate'])) {
			// only use the raw tree value in the hash, generate from assetid dynamically
			$hash->setLinkTree($GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($settings['assetid']), $settings['propagate']);
		}

		return FALSE;

	}//end setHash()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* Default to false, children can override if necessary
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{
		// default to to disallow
		return TRUE;

	}//end allowMultiple()


}//end class

?>
