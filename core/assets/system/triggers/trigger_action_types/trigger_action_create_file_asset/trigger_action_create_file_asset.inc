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
* $Id: trigger_action_create_file_asset.inc,v 1.14 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Create File Asset
*
* A trigger action that creates a new file-based asset
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_File_Asset extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'parentid'     => string  // the ID of the new asset's parent
	*       'type_code'    => string  // the type of the new asset
	*       'link_type'    => int     // the type of link to attach to the parent
	*       'value'        => string  // the value associated with a link (optional)
	*       'is_dependant' => boolean // whether the link is dependant (optional)
	*       'is_exclusive' => boolean // whether the link is exclusive (optional)
	*       'file_path'	   => string  // path to the file to use
	*       'file_name'	   => string  // a name to use for the file (optional)
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
		// check required settings
		if (empty($settings['type_code'])) return FALSE;
		if (empty($settings['parentid'])) return FALSE;
		if (empty($settings['link_type'])) return FALSE;
		if (empty($settings['file_path'])) return FALSE;
		if (!is_readable($settings['file_path'])) {
			trigger_localised_error('CORE0276', E_USER_WARNING, htmlentities($settings['file_path'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
			return FALSE;
		}

		if ($settings['parentid'] == 'current_asset') {
			$settings['parentid'] = $state['assetid'];
		}
		$attributes = Array();
		if (!empty($settings['attributes'])) {
			$attributes = $settings['attributes'];
		}
		$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['parentid']);
		if (is_null($parent)) return FALSE;

		// check the optional settings, assign defaults
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);
		$value = array_get_index($settings, 'value', '');

		// the new link
		$link = Array(
					'asset'			=> &$parent,
					'link_type'		=> $settings['link_type'],
					'value'			=> $value,
					'sort_order'	=> NULL,
					'is_dependant'	=> (bool) $is_dependant,
					'is_exclusive'	=> (bool) $is_exclusive,
				);

		if (isset($settings['file_name']) === false) {
			$settings['file_name'] = basename($settings['file_path']);
		}

		$info = Array(
					'name'				=> $settings['file_name'],
					'tmp_name'			=> $settings['file_path'],
					'non_uploaded_file'	=> TRUE,
				);

		$type_code = $settings['type_code'];
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$new_asset = new $type_code();
		$new_asset->_tmp['uploading_file'] = TRUE;

		// set the attributes for the newly created asset
		if (!empty($attributes)) {
			foreach ($attributes as $index => $val) {
				$new_asset->setAttrValue($val['attribute'], $val['value']);
			}
		}

		$linkid = $new_asset->create($link, $info);
		if (!$linkid) return FALSE;

		// If we're running at a low runlevel, we need to run the _createAdditional function here
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
			if (!$new_asset->createAdditional($link)) {
				return FALSE;
			}

		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

		return Array(
				'assetid'		=> $link['minorid'],
				'parentid'		=> $link['majorid'],
				'linkid'		=> $link['linkid'],
				'link_type'		=> $link['link_type'],
				'value'			=> $link['value'],
				'sort_order'	=> $link['sort_order'],
				'is_dependant'	=> $link['is_dependant'],
				'is_exclusive'	=> $link['is_exclusive'],
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
		// check settings, set defaults if necessary
		$parentid     = array_get_index($settings, 'parentid', 0);
		$type_code    = array_get_index($settings, 'type_code', 0);
		$link_type    = array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);
		$value        = array_get_index($settings, 'value', '');
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);
		$selected     = array_get_index($settings, 'attributes', 0);

		$attrs = Array();
		$attributes = Array();
		if (!empty($type_code)) {
			// get the attributes of this asset
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type_code);
			foreach ($attrs as $attr => $type) {
				$attributes[$attr] = ucwords(str_replace('_', ' ', $attr)).' - '.ucwords($type['type']);
			}
			$attributes[''] = '--SELECT ATTRIBUTE--';
		}

		// check if the parent asset is valid
		$parent_broadcasted = FALSE;
		if ($parentid == 'current_asset') {
			$parent_broadcasted = TRUE;
			$parentid = 0;
		} else {
			if ($parentid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($parentid)) {
				$parentid = 0;
			}
		}

			// begin buffering basic options
			ob_start();
				if ($write_access) {
					asset_type_chooser($prefix.'[type_code]', FALSE, Array($type_code), TRUE, FALSE, FALSE, Array('file'));
				} else {
					echo '<b>'.$type_code.'</b>';
				}

				$basic_part_1 = ob_get_contents();
			ob_end_clean();


			ob_start();
				if ($write_access) {
					echo '<ul class="sq-radio-list">';
						echo '<li>';
							radio_button($prefix.'[parent_type]', 'explicit', !$parent_broadcasted, '', 'id="parent_type_explicit"');
							label(strtolower(translate('asset')), 'parent_type_explicit');
							echo ' ';
							asset_finder($prefix.'[parentid]', $parentid, Array('asset' => 'D'));
						echo '</li>';
						echo '<li>';
							radio_button($prefix.'[parent_type]', 'broadcaster', $parent_broadcasted, '', 'id="parent_type_broadcaster"');
							label(translate('trigger_create_under_broadcaster'), 'parent_type_broadcaster');
						echo '</li>';
					echo '</ul>';
				} else {
					if ($parent_broadcasted) {
						echo '<b>'.translate('trigger_create_under_broadcaster').'</b>';
					} else {
						echo translate('under').' ';
						if (!empty($parentid)) {
							echo '<b>'.get_asset_tag_line($parentid).'</b>';
						} else {
							echo '<b>['.translate('no_parent_selected').']</b>';
						}
					}
				}

				$basic_part_2 = ob_get_contents();
			ob_get_clean();

			ob_start();
				$link_type_list = get_link_type_names();
				unset($link_type_list[SQ_LINK_NOTICE]);
				if ($write_access) {
					combo_box($prefix.'[link_type]', $link_type_list, FALSE, $link_type);
				} else {
					echo '<b>'.$link_type_list[$link_type].'</b>';
				}

				$basic_part_3 = ob_get_contents();
			ob_end_clean();

			ob_start();
				if ($write_access) {
					text_box($prefix.'[file_path]', array_get_index($settings, 'file_path', ''), 40);
				} else {
					echo array_get_index($settings, 'file_path', '');
				}
				$basic_part_4 = ob_get_contents();
			ob_end_clean();

			// begin buffering advanced options
			ob_start();
				if ($write_access) {
					text_box($prefix.'[value]', $value, '', '');
				} else {
					if (empty($value)) {
						$value = '['.translate('no_value_entered').']';
					}
					echo '<b>'.$value.'</b>';
				}
				$advanced_part_1 = ob_get_contents();
			ob_end_clean();

			ob_start();
				$is_dependant_list = Array(
										0	=> translate('non_dependently'),
										1	=> translate('dependently'),
									 );
				if ($write_access) {
					combo_box($prefix.'[is_dependant]', $is_dependant_list, FALSE, $is_dependant);
				} else {
					echo '<b>'.$is_dependant_list[$is_dependant].'</b>';
				}

				$advanced_part_2 = ob_get_contents();
			ob_end_clean();

			ob_start();
				$is_exclusive_list = Array(
										0	=> translate('non_exclusively'),
										1	=> translate('exclusively'),
									 );
				if ($write_access) {
					combo_box($prefix.'[is_exclusive]', $is_exclusive_list, FALSE, $is_exclusive);
				} else {
					echo '<b>'.$is_exclusive_list[$is_exclusive].'</b>';
				}

				$advanced_part_3 = ob_get_contents();
			ob_end_clean();

			ob_start();
			if (!empty($attributes)) {
				$count = 0;
				if (!empty($selected)) {
					$count = count($selected);
					foreach ($selected as $index => $attr) {
						if ($write_access) {
							combo_box($prefix.'[attributes]['.$index.'][attribute]', $attributes, FALSE, $attr['attribute']);
							echo '&nbsp;';
							text_box($prefix.'[attributes]['.$index.'][value]', $attr['value'], 50, '', FALSE, 'style="margin-bottom: 3px;"');
							echo '&nbsp;';
							check_box($prefix.'[attributes]['.$index.'][delete]','delete', FALSE);
							label(translate('delete_question'), $prefix.'[attributes]['.$index.'][delete]');
							echo '<br />';
						} else {
							echo ucwords($attr['attribute']).':  '.$attr['value'].'<br />';
						}
					}
				} else {
					if (!$write_access) {
						echo '<i>('.translate('none').')</i>';
					}
				}
				if ($write_access) {
					combo_box($prefix.'[attributes]['.$count.'][attribute]', $attributes, FALSE, '');
					echo '&nbsp;';
					text_box($prefix.'[attributes]['.$count.'][value]', '', 50);
					echo '<br />';
				}
			}
			$attributes = ob_get_contents();
			ob_end_clean();

			// now patch together the output
			$contents =	translate('trigger_new_asset_basic', $basic_part_1, $basic_part_2, $basic_part_3).
						' and populate it with the file located on the server at <strong>'.$basic_part_4.'</strong>.'.
						'<br /><br />'.
						'<b>'.translate('triggers_advanced_user_options').'</b><br />'.
						translate('trigger_new_asset_advanced', $advanced_part_1, $advanced_part_2, $advanced_part_3).
						'<br /><br /><b>'.translate('set_attributes').'</b><br />'.$attributes.'<br /><br />'.
						'<span>'.translate('trigger_create_file_required_attrs_warning').'</span>';

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
		// make sure the parentid isn't blank
		if (($request_data['parent_type'] != 'broadcaster') && empty($request_data['parentid']['assetid'])) {
			return translate('parent_to_create_under_is_missing');
		}

		if (!isset($request_data['file_path']) || empty($request_data['file_path'])) {
			return translate('trigger_action_create_file_asset_file_path_not_set');
		}

		$link_type_list = get_link_type_names();
		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return translate('link_type_missing');
		}

		if (($request_data['parent_type'] == 'explicit')) {
			$settings['parentid']  = $request_data['parentid']['assetid'];
		} else {
			$settings['parentid']  = 'current_asset';
		}
		$settings['link_type'] = $request_data['link_type'];

		$type_code = array_get_index($request_data, 'type_code', FALSE);
		if (!$type_code) {
			return translate('asset_type_not_specified');
		}

		$settings['type_code'] = $type_code;
		$settings['file_path'] = $request_data['file_path'];

		// optional fields
		$settings['value']        = array_get_index($request_data, 'value', '');
		$settings['is_dependant'] = array_get_index($request_data, 'is_dependant', 0);
		$settings['is_exclusive'] = array_get_index($request_data, 'is_exclusive', 0);

		// if the user has chosen attributes and their default values
		if (isset($request_data['attributes']) && !empty($request_data['attributes'])) {
			$attribute = $request_data['attributes'];
			$new_attr = Array();
			foreach ($attribute as $index => $val) {
				if (empty($val['attribute']) || empty($val['value'])) {
					continue;
				}
				if (isset($val['delete'])) continue;
				$new_attr[] = $val;
			}
			$settings['attributes'] = $new_attr;
		}
		return FALSE;

	}//end processInterface()


}//end class

?>
