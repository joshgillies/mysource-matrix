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
* $Id: trigger_action_remove_links.inc,v 1.6 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Remove Links Trigger Action
*
* A trigger action that removes a link between the broadcasting asset and another
* asset as specified by one or more of the following:
* 	- Linked asset resides under Root Node (can also be supplied from a Parameter Map)
* 	- Linked Asset Type
* 	- Link Type
* 	- Link Value
*	- Direct Links Only
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Links extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'assetid'						=> int,    // the assetid to link to
	*       'is_major'						=> int,    // 0 = firing asset on minor side, 1 = ..on major side
	*       'link_type'						=> string, // the type of link (or 'ANY' for all),
	*       'link_value'					=> string, // the link value,
	*       'any_link_value'				=> boolean,// whether we are interested in any link value as opposed to a blank one,
	*       'direct_links_only'				=> boolean,// whether to look for and remove links directly under the root node (TRUE)
	*												   // or to look under the root asset (FALSE) for candidate links
	*       'asset_types'					=> array,  // the matching asset types of the linked asset
	*       'parameter_map_target_asset'	=> string, // serialisation of parameter map settings (trigger view only)
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check for required settings
		if (!isset($settings['is_major']) || empty($settings['link_type'])) {
			return FALSE;
		}

		$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
		$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$map_assetid = $attr_parameter_map->getParameterValue('asset');
		$direct_links_only = $attr_parameter_map->getParameterValue('direct_links_only');

		if (empty($settings['assetid']) && empty($map_assetid)) {
			return FALSE;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return FALSE;
		}

		if (!is_null($map_assetid) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($map_assetid)) {
			return FALSE;
		}

		// check the optional settings, assign defaults
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);
		$link_value = array_get_index($settings, 'link_value', '');

		if (empty($state['asset'])) {
			// if we've been given the assetid but not the asset, get the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// whether we use the parameter map value or the settings value for the target asset
		$assetid = is_null($map_assetid) ? $settings['assetid'] : $map_assetid;

		$major = $am->getAsset($assetid);
		$minor = $state['asset'];

		$link_value_sql = (!empty($settings['link_value'])) ? 'l.value = '.MatrixDAL::quote($settings['link_value']).' AND ' : '';

		// If the Link Value is set to blank and we haven't selected "Any Link Value", then we want empty link values only
		if (empty($link_value_sql) && (!$settings['any_link_value'])) {
			$link_value_sql = 'l.value = '.MatrixDAL::quote('').' AND ';
		}

		$link_type_sql = ($settings['link_type'] != 'ANY') ? 'l.link_type = '.MatrixDAL::quote($settings['link_type']).' AND ' : '';
		$link_asset_type_codes_sql = '';

		if (!empty($settings['asset_types'])) {
			$link_asset_type_codes_sql = 'AND a.type_code IN (';
			foreach ($settings['asset_types'] as $asset_type) {
				$link_asset_type_codes_sql .= MatrixDAL::quote($asset_type).',';
			}
			// Remove last comma and terminate the IN clause
			$link_asset_type_codes_sql = substr($link_asset_type_codes_sql, 0, strlen($link_asset_type_codes_sql)-1).')';
		}

		$major_or_minor_link = 'minorid';

		$broadcasting_asset_id = $state['assetid'];
		$root_node_asset_id = $assetid;

		// Compile a list of candidate links which reside DIRECTLY under our Root Node (default operation)
		$assets = 'SELECT DISTINCT 
						a.assetid
					FROM
						sq_ast a, sq_ast_lnk l
					WHERE
						l.majorid = '.MatrixDAL::quote($root_node_asset_id).' AND
						a.assetid = l.minorid AND
						a.assetid != '.MatrixDAL::quote($broadcasting_asset_id).' '.
						$link_asset_type_codes_sql;

		// Find out whether the Parameter Map was used to populate the "Direct Links Only" option - if so, populate our retrieved value
		$direct_links_only = is_null($direct_links_only) ? $settings['direct_links_only'] : ($direct_links_only == 1);

		// If we are looking in the entire tree (ie; not "Direct Links Only"), generate a query to compile a list of candidate links which reside anywhere under our Root Node
		if (!$direct_links_only) {
			// Select candidate links from under the Root Node
			// Get the Tree ID of our Root Node
			$tree_id_sql = 'SELECT
								t.treeid
							FROM
								sq_ast_lnk l, sq_ast_lnk_tree t
							WHERE
								t.linkid = l.linkid AND
								l.minorid = '.MatrixDAL::quote($root_node_asset_id);

			$root_node_tree_id = MatrixDAL::executeSqlOne($tree_id_sql);

			// Ok, any matching assets should be linked under the Broadcasting Asset back to underneath our Root Node (ie; share a common Tree ID)
			// Let's climb the tree...
			$assets = 'SELECT DISTINCT 
							a.assetid
						FROM
							sq_ast a, sq_ast_lnk l, sq_ast_lnk_tree t
						WHERE
							t.linkid = l.linkid AND
							t.treeid LIKE '.MatrixDAL::quote($root_node_tree_id.'%').' AND
							t.treeid != '.MatrixDAL::quote($root_node_tree_id).' AND
							a.assetid = l.minorid AND
							a.assetid != '.MatrixDAL::quote($broadcasting_asset_id).' '.
							$link_asset_type_codes_sql;
		}

		// Abort if we're about to remove the last significant link!
		if ($settings['link_type'] & SQ_SC_LINK_SIGNIFICANT) {
			$our_links = $am->getLinks($broadcasting_asset_id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
			$num_links = count($our_links);

			// Abort... abort... abort...
			if ($num_links < 2) return Array();
		}

		// Match candidate links with links associated with (ie; immediately above) the Broadcasting Asset
		$sql = 'SELECT
					l.linkid
				FROM
					sq_ast_lnk l
				WHERE
					l.minorid = '.MatrixDAL::quote($broadcasting_asset_id).' AND '.
					$link_type_sql.
					$link_value_sql.
					'l.majorid IN ('.
						$assets.
					')';

		// Our links to remove
		$old_links = MatrixDAL::executeSqlAssoc($sql);

		// Remove the candidate links
        $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
        $db = MatrixDAL::getDb();
        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			foreach ($old_links as $old_link) {
				$am->deleteAssetLink($old_link['linkid']);
			}

            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

        } catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
            throw new Exception('Unable to delete links for asset: '.addslashes($assetid).' due to database error: '.$e->getMessage());
        }

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// update lookups for minor assets
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($minor->id));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);
		$major->updateLookups();
		
		return Array();

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
		// check settings, set defaults if necessary
		$selected_assetid   = array_get_index($settings, 'assetid', 0);
		$is_major           = array_get_index($settings, 'is_major', 1);
		$selected_link_type = array_get_index($settings, 'link_type', 'ANY');
		$link_value         = array_get_index($settings, 'link_value', '');
		$is_exclusive       = array_get_index($settings, 'is_exclusive', 0);
		$any_link_value     = array_get_index($settings, 'any_link_value', TRUE);
		$direct_links_only	= array_get_index($settings, 'direct_links_only', TRUE);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$is_major_list  = Array(0 => 'Child', 1 => 'Parent');

		$link_type_list = get_link_type_names();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// Matching Asset Types
        $asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
        foreach ($asset_types as $type_code) {
            $info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
            if ($info['allowed_access'] != 'system' && $info['instantiable']) {
                $asset_types_options[$type_code] = $info['name'];
            }
        }
        asort($asset_types_options);
        $asset_types_options = array_reverse($asset_types_options, TRUE);
        $asset_types_options[''] = '-- '.translate('all_asset_types').' --';
        $asset_types_options = array_reverse($asset_types_options, TRUE);

        // initialise settings var
        $settings['asset_types'] = isset($settings['asset_types']) ? $settings['asset_types'] : Array();
        if (empty($settings['asset_types'])) {
            $settings['asset_types'] = Array(0 => '');
        }

		ob_start();
				?><table><tr><?php

					if ($write_access) {
						?>
						<td valign="top"><b><?php echo translate('trigger_remove_link_matching_asset_types'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php echo combo_box($prefix.'[asset_types]', $asset_types_options, TRUE, $settings['asset_types'], 8); ?></td>
						<?php
					} else {
						?>
						<td valign="top"><b><?php echo translate('trigger_remove_link_matching_asset_types'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php
							if (!empty($settings['asset_types'][0])) {
								?><ul><?php
								foreach ($settings['asset_types'] as $type_code) {
									?><li><?php
									echo get_asset_type_icon($type_code);
									echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
									?></li><?php
								}
								?></ul><?php
							} else {
								echo '&nbsp; &nbsp;'.translate('all_asset_types');
							}
						?></td><?php
					}

				?></tr></table><?php

			$asset_types_combo_box = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), TRUE, 'name');
					text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', FALSE, $form_element_extras);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$link_type_list['ANY'] = 'ANY TYPE';

			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
			$basic_part_3 = ob_get_contents();
		ob_end_clean();


		// attribute friendly prefix
		$new_prefix = str_replace(array('[', ']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// parameter map options
		ob_start();
			$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
			$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$attr_parameter_map->setParameter('asset', 'Candidate Assets');
			$attr_parameter_map->setParameter('direct_links_only', 'Where Candidate Assets are Directly Linked to the Root Node (1 = TRUE)');
			echo $attr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
			$basic_part_4 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$link_value_onkeypress = '';
			$any_link_value_onclick = '';

			if ($write_access) {
				$link_value_onkeypress = 'onkeypress = "document.getElementById(\'any_link_value\').checked = false;"';
				$any_link_value_onclick = 'onclick = "document.getElementById(\'link_value\').value = \'\';"';
			}

			text_box($prefix.'[link_value]', $link_value, '', '', FALSE, $form_element_extras.' '.$link_value_onkeypress.' id="link_value"');
			check_box($prefix.'[any_link_value]', '1', $any_link_value, '', $form_element_extras.' '.$any_link_value_onclick.' id="any_link_value"');
			echo translate('trigger_remove_link_any_link_value');

			echo '<p>'.translate('trigger_remove_link_direct_links_only');
			check_box($prefix.'[direct_links_only]', '1', $direct_links_only, '', $form_element_extras);
			echo '</p>';

			$advanced_part_1 = ob_get_contents();
		ob_end_clean();

		$contents = translate('trigger_remove_link_basic', $basic_part_2).'<br />'.
					translate('trigger_remove_link_advanced', $basic_part_3, $advanced_part_1).'<br /><br />'.
					$asset_types_combo_box.
					$basic_part_4;

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
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
		$is_major_list = Array(0 => 'Child', 1 => 'Parent');

		$link_type_list = get_link_type_names();
		$link_type_list['ANY'] = 'ANY TYPE';

		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return 'Asset ID not specified';
		}

		$request_data['is_major'] = TRUE;
		if (!isset($request_data['is_major']) || !isset($is_major_list[$request_data['is_major']])) {
			return 'Parent or Child link not specified';
		}

		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return 'Link Type not specified';
		}

		$new_prefix = str_replace(array('[', ']'), '_', array_get_index($request_data, 'new_prefix', ''));
		$attr_parameter_map = new Asset_Attribute_Parameter_Map();
		$attr_parameter_map->process($new_prefix.'_parameter_map');
		$settings['parameter_map_target_asset'] = $attr_parameter_map->value;

		$settings['assetid']   = $request_data['assetid']['assetid'];
		$settings['link_id']   = $request_data['assetid']['linkid'];
		$settings['is_major']  = $request_data['is_major'];
		$settings['link_type'] = $request_data['link_type'];

		// selected asset types
		$settings['asset_types'] = Array();
		if (isset($request_data['asset_types']) && !empty($request_data['asset_types'][0])) {
			$settings['asset_types'] = $request_data['asset_types'];
		}

		// optional fields
		$settings['link_value']			= array_get_index($request_data, 'link_value', '');
		$settings['any_link_value']		= array_get_index($request_data, 'any_link_value', FALSE);
		$settings['direct_links_only']	= array_get_index($request_data, 'direct_links_only', FALSE);

		return FALSE;

	}//end processInterface()


}//end class

?>
