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
* $Id: trigger_action_create_asset.inc,v 1.17 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Create Asset
*
* A trigger action that creates a clone of the firing asset
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Asset extends Trigger_Action
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

		$type_code = $settings['type_code'];
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$new_asset = new $type_code();
		// set the attributes for the newly created asset
		if (!empty($attributes)) {
			foreach ($attributes as $index => $val) {
				$value = $val['value'];
				//if the replace keywords checkbox is checked, do keyword replacement here before setting the value to attribute
				if (array_get_index($settings, 'replace_keywords', 0)) {
					// replace global keywords
					$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

					replace_global_keywords($value);

					if (is_null($old_current_asset)) {
						$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
					} else {
						$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
					}

					// replace broadcasting asset keywords
					$keywords = retrieve_keywords_replacements($value);
					$replacements = Array();
					foreach ($keywords as $keyword) {
						$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
					}
					replace_keywords($value, $replacements);
				}
				$new_asset->setAttrValue($val['attribute'], $value);
			}
		}
		$linkid = $new_asset->create($link);
		if (!$linkid) return FALSE;

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
					asset_type_chooser($prefix.'[type_code]', FALSE, Array($type_code), TRUE);
				} else {
					echo '<b>'.$type_code.'</b>';
				}

				$basic_part_1 = ob_get_contents();
			ob_end_clean();


			ob_start();
				if ($write_access) {
					echo '<br />';
					check_box($prefix.'[parentid]', 'current_asset', $parent_broadcasted);
					label(translate('trigger_create_under_broadcaster'), $prefix.'[parentid]');
					echo '<br /><b> OR </b><br />';
					asset_finder($prefix.'[parentid]', $parentid, Array('asset' => 'D'));
				} else {
					$parent = NULL;
					if ($parentid > 0) {
						$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
					}
					if (is_null($parent)) {
						echo '<b>['.translate('no_asset_selected').']</b>';
					} else {
						echo get_asset_tag_line($parent->id);
					}
				}

				$basic_part_2 = ob_get_contents();
			ob_get_clean();

			$link_type_list = get_link_type_names();
			unset($link_type_list[SQ_LINK_NOTICE]);

			ob_start();
				if ($write_access) {
					combo_box($prefix.'[link_type]', $link_type_list, FALSE, $link_type);
				} else {
					echo '<b>'.$link_type_list[$link_type].'</b>';
				}

				$basic_part_3 = ob_get_contents();
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
							echo '<strong>'.ucwords($attr['attribute']).':</strong>  '.$attr['value'].'<br />';
						}
					}
				}
				if ($write_access) {
					combo_box($prefix.'[attributes]['.$count.'][attribute]', $attributes, FALSE, '');
					echo '&nbsp;';
					text_box($prefix.'[attributes]['.$count.'][value]', '', 50);
				}
			}
			$attributes = ob_get_contents();
			ob_end_clean();
			
			// Replace keywords checkbox
			ob_start();
				check_box($prefix.'[replace_keywords]', '1', array_get_index($settings, 'replace_keywords', 0), NULL, ($write_access? '' : 'disabled="disabled"'));
				label('Replace keywords in the attribute value', $prefix.'[replace_keywords]');
			$replace_keywords = '<br />'.ob_get_clean();

			$attributes .= $replace_keywords;

			// now patch together the output
			$contents =	translate('trigger_new_asset_basic', $basic_part_1, $basic_part_3).
						'<br /><br />'.
						'<b>'.translate('create_asset_under').'</b>'.$basic_part_2.
						'<br /><br /><b>'.translate('triggers_advanced_user_options').'</b><br />'.
						translate('trigger_new_asset_advanced', $advanced_part_1, $advanced_part_2, $advanced_part_3).
						'<br /><br /><b>'.translate('set_attributes').'</b><br />'.$attributes.'<br /><br />'.
						'<span>'.translate('trigger_create_asset_required_attrs_warning').'</span>';

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
		if (($request_data['parentid'] != 'current_asset') && empty($request_data['parentid']['assetid'])) {
			return translate('parent_to_create_under_is_missing');
		}

		$link_type_list = get_link_type_names();
		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return translate('link_type_missing');
		}

		if (is_array($request_data['parentid'])) {
			$settings['parentid']  = $request_data['parentid']['assetid'];
		} else {
			$settings['parentid']  = $request_data['parentid'];
		}
		$settings['link_type'] = $request_data['link_type'];

		$type_code = array_get_index($request_data, 'type_code', FALSE);
		if (!$type_code) {
			return translate('asset_type_not_specified');
		}
		$settings['type_code'] = $type_code;

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
		
		$settings['replace_keywords']      = array_get_index($request_data, 'replace_keywords', 0);
		
		return FALSE;

	}//end processInterface()


	/**
	* Add the attributes value from the import script to a settings array
	*
	* Attributes should be specified in the actions in the following format:
	* <attributes>
	*   <attribute>
	*     <name>NAME OF ATTRIBUTE</name>
	*     <value>ATTRIBUTE TO SET</value>
	*   </attribute>
	*   (...more attributes...)
	* </attributes>
	*
	* Currently, only simple attributes (non-serialised) can be set.
	*
	* @param string	$value		The a value to be added to the settings array
	* @param array	$settings	a container for any data the action might want to save
	*
	* @return array
	* @access public
	*/
	public static function addSettingAttributes($value, $settings)
	{
		foreach ($value['attribute'] as $attribute) {
			$attr_name = $attribute['name'][0];
			$attr_value = $attribute['value'][0];
			$settings['attributes'][] = Array(
											'attribute'	=> $attr_name,
											'value'		=> $attr_value,
										);
		}

		return $settings;

	}//end addSettingAttributes()


}//end class

?>
