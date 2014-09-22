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
* $Id: trigger_condition_tree_location.inc,v 1.22 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Condition Tree Location
*
* Checks whether an asset is under a particular branch of a tree
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @author	Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.22 $
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
	*       'assetids'       => array(int),							// either assetids or treeids needs to be set. the asset ids of the parent assets
	*       'treeids'        => array(assetid => array(string)),	// can be retrieved from assetids if necessary. the raw tree ids of the parent assets
	*       'immed_child'	 => array(assetid => int),				// 1 or 0. determines whether the asset being checked is supposed to be an immediate child, or any child
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

		if (empty($settings['treeids'])) {
			if (empty($settings['assetids'])) {
				return FALSE;
			} else {
				$settings['treeids'] = Array();
				foreach ($settings['assetids'] as $tree_asset_id) {
					if ($GLOBALS['SQ_SYSTEM']->am->assetExists($tree_asset_id)) {
						$settings['treeids'][$tree_asset_id] = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($tree_asset_id);
					}
				}
			}
		}

		if (empty($settings['immed_child'])) return FALSE;

		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['treeid'])) {
			// not stored, need to retrieve
			$state['treeid'] = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($assetid);
			if (empty($state['treeid'])) trigger_localised_error('TRIG0006', E_USER_WARNING);
		}

		if (!is_array($state['treeid'])) {
			$state['treeid'] = Array($state['treeid']);
		}

		// evaluate the condition
		foreach ($state['treeid'] as $state_tree) {
			foreach ($settings['treeids'] as $tree_asset_id => $treeids) {
				foreach ($treeids as $settings_tree) {
					// if the asset in question is under the tree specified in the settings...
					if ($settings_tree === $state_tree) {
						break;
					} else {
						if ($settings['immed_child'][$tree_asset_id]) {
							if (strlen($settings_tree) == strlen($state_tree) - SQ_CONF_ASSET_TREE_SIZE &&
							substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
								return TRUE;
							}
						} else {
							if (substr($state_tree, 0, strlen($settings_tree)) == $settings_tree) {
								return TRUE;
							}
						}
					}//end if
				}
			}
		}

		return FALSE;

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
		$selected_assetids = array_get_index($settings, 'assetids', Array());

		// check if the selected assets are valid
		foreach ($selected_assetids as $selected_assetid){
			if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
				unset($selected_assetids[$selected_assetid]);
			}
		}

		$immed_child = array_get_index($settings, 'immed_child', Array());

		// begin buffering basic options
		ob_start();
			// create an asset_finder, and pre-select the asset that is saved in settings (if any)
			if ($write_access) {
				?><script type="text/javascript" src="<?php echo sq_web_path('lib')?>/html_form/html_form.js"></script><?php
				echo '<div class="sq-multiple-asset-finder-wrapper" style="margin-top: 5px;">';
				$name = $prefix.'[assetids]';
				foreach (array_values($selected_assetids) as $index => $selected_assetid){
					asset_finder($name.'['.$index.']', $selected_assetid, Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
					echo '<span style="margin-left:10px;">';
					echo translate('Immediate child only');

					check_box($prefix.'[immed_child]['.$index.']', '1', (!empty($immed_child[$selected_assetid])), '', 'style="margin-left:10px;"');
					echo '</span>';
					echo '<br />';
				}

				asset_finder($name.'['.count($selected_assetids).']', 0, Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
				echo '<span id="trigger_condition_tree_location_check_box" style="margin-left:10px;">';
				echo translate('Immediate child only');

				check_box($prefix.'[immed_child]['.count($selected_assetids).']', '1', FALSE, '', 'style="margin-left:10px;"');
				echo '</span>';

				$map_frame_exp = trim('parent.frames["sq_sidenav"]', '.');
				$safe_name = 'sq_asset_finder_'.preg_replace('/[^A-Za-z0-9]/', '_', $name);

				?>
				<script type="text/javascript">
				// insert target parent check box when clicking More button
				var immed_child_index = <?php echo count($selected_assetids)+ 1; ?>;
				function addParentNode(node) {
					var new_span = document.getElementById('trigger_condition_tree_location_check_box').cloneNode(true);
					new_span.removeAttribute('id');
					new_span.lastChild.setAttribute('id', '<?php echo $prefix; ?>' + '[immed_child][' + immed_child_index + ']');
					new_span.lastChild.setAttribute('name', '<?php echo $prefix; ?>' + '[immed_child][' + immed_child_index + ']');
					node.parentNode.insertBefore(new_span, node);
					immed_child_index = immed_child_index + 1;
				}
				</script>
				<?php

				echo '<input type="button" name="'.htmlspecialchars($safe_name.'_more_btn').'" id="'.htmlspecialchars($safe_name.'_more_btn').'" value="'.translate('More').'..." onclick="addNewAssetFinder(this, \''.$name.'\', \''.$safe_name.'\', \'\', \''.htmlentities($map_frame_exp, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'\', \'null\', true); addParentNode(this);" style="margin-left:10px;"/>';

				echo '<br />';
				echo '</div>';
			} else {
				if ($selected_assetids) {
					echo '<div class="sq-root-node-selections-wrapper">';
					foreach ($selected_assetids as $selected_assetid){
						echo get_asset_tag_line($selected_assetid);
						if (!empty($immed_child[$selected_assetid])) {
							echo '<b> '.translate('as an immediate child only').'</b>';

						}
						echo '<br />';
					}
					echo '</div>';
				} else {
					echo '<b>['.translate('No asset selected').']</b>';

				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('Asset is under any one of these locations: <br />%s'), $basic_part_1);


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
		$settings['assetids'] = $settings['propagate'] = $settings['immed_child'] = Array();
		$immed_child = array_get_index($request_data, 'immed_child', Array());
		$assetids = array_get_index($request_data, 'assetids', Array());
		foreach ($assetids as $index => $tree_location){
			if (!empty($tree_location['assetid'])) {
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($tree_location['assetid'])) {
					$settings['assetids'][] = $tree_location['assetid'];
					if (empty($immed_child[$index])) {
						$settings['propagate'][$tree_location['assetid']] = SQ_TRIG_TREE_PROPAGATE;
						$settings['immed_child'][$tree_location['assetid']] = '0';
					} else {
						$settings['propagate'][$tree_location['assetid']] = SQ_TRIG_TREE_NO_PROPAGATE;
						$settings['immed_child'][$tree_location['assetid']] = '1';
					}
				} else {
					trigger_error(translate('Selected asset does not exist'), E_USER_NOTICE);

				}
			}
		}

		if (empty($settings['assetids'])) return translate('No tree location selected');


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
		if (isset($settings['assetids']) && isset($settings['propagate'])) {
			// only use the raw tree value in the hash, generate from assetid dynamically
			foreach ($settings['assetids'] as $assetid){
				$hash->setLinkTree($GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($assetid), $settings['propagate'][$assetid]);
			}
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
