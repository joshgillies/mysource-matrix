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
* $Id: page_asset_builder_management.inc,v 1.12 2004/08/19 02:16:36 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Builder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Asset_Builder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		/**
		* Array of asset type codes that this page can create
		*/
		$this->vars['create_type']				= Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(),
												);

		$this->vars['create_status']			= Array(
													'added'      => '0.1',
													'type'       => 'int',
													'default'    => 2,
												);
															
		/**
		* Locations under which created assets will always be linked
		*	Array (assetid => link_type)
		*/
		$this->vars['create_location']			= Array(
															'added'     => '0.1',
															'type'      => 'serialise',
															'default'   => Array(),
														);
														
		

		/**
		* Sets of locations for the user to choose from when creating assets, with selection options
		*	Array (name => Array('locations'		=> Array (assetid => name),
		*						  'allow_multiple'	=> 0 | 1,
		*						  'allow_empty'		=> 0 | 1,
		*						  'input_style'		=> 'dropdown' | 'radio',
		*						  'link_type'		=> link_type
		*						 )
		*		  )
		*/
		$this->vars['create_location_optional']	= Array(
															'added'     => '0.1',
															'type'      => 'serialise',
															'default'   => Array(),
														);
		/**
		* Rules governing additional create locations
		*   Array (create_type => Array (index => Array('attributes'  => Array(index => attribute_name),
		*												'comparators' => Array(index => comparator_name),
		*												'values'	  => Array(index => comparison_value),
		*
		*												'locations'	  => Array(index => assetid),
		*												'link_types'  => Array(index => link_type)
		*											   )
		*								)
		*		  )
		*/
		$this->vars['create_location_rules']			= Array(
															'added'     => '0.1',
															'type'      => 'serialise',
															'default'   => Array(),
														);

		/**
		* WYSIWYGs
		*/
		$this->vars['login_invite']						= Array(
															'added'     => '0.1',
															'type'      => 'wysiwyg',
															'default'   => '<p>Sign in using the form below.</p>',
														);
		$this->vars['login_error']						= Array(
															'added'     => '0.1',
															'type'      => 'wysiwyg',
															'default'   => '<p>Incorrect username or password entered. Please try again.</p>',
														);
		$this->vars['create_invite']					= Array(
															'added'     => '0.1',
															'type'      => 'wysiwyg',
															'default'   => '<p>Create an asset using the form below.</p>',
														);
		$this->vars['create_error']						= Array(
															'added'     => '0.1',
															'type'      => 'wysiwyg',
															'default'   => '<p>The new asset was not created. Ensure you have entered all required information and try again.</p>',
														);

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// important upgrades that must be taken care of before create type becomes
		// a serialise in 0.3
		if ($current_version <= '0.2') {
			$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) { 
					trigger_error('Asset Builder Upgrade [from 0.2] Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					return false;
				}
				
				$old_create_type = $asset->attr('create_type');
				$create_type = Array($old_create_type => true);
				$asset->setAttrValue('create_type', serialize($create_type));
								
				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade [from 0.2] Failed', _USER_WARNING);
					return false;
				}
				
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
			
			}
		}								
				
		
		if (!parent::_upgrade($current_version)) return false;

		if ($current_version == '0.1') {
			// this upgrade changed the attribute 'create_locations' to include additional
			// rules that meant the create locations has to be stored in a different format

			$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) { 
					trigger_error('Asset Builder Upgrade Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					return false;
				}

				// 0.1->0.2 upgrades
				$new_locations = Array();
				$old_locations = $asset->attr('create_location_rules');
				
				$i = 0;
				foreach ($old_locations as $rule_text => $locations) {
					if (empty($rule_text)) continue;
					$bits = explode(' && ', $rule_text);
	
					foreach ($bits as $bit) {
						list($attr, $value) = explode('==', $bit);
						$attr  = trim($attr, '$');
						$value = trim($value, " '");
						$new_locations[$i][$attr]['equals'] = $value;
					}
					
					$new_locations[$i]['_create_locations_'] = array_keys($locations);
					
					$i++;
				}
				
				$asset->setAttrValue('create_location_rules', $new_locations);				
				
				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade Failed', E_USER_WARNING);
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes'); 
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			
			bam('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.1');
		}//end version 0.1 upgrade

		if ($current_version == '0.2') {
			// this update changed the way create types are stored, to allow for the creation
			// of multiple asset types
			bam('STARTING ASSET BUILDER UPGRADE - VERSION 0.2');

			$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) { 
					trigger_error('Asset Builder Upgrade [from 0.2] Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					return false;
				}
				
				$create_locs = $asset->attr('create_location_rules');
				$create_locs = Array($old_create_type => $create_locs);
								
				$asset->setAttrValue('create_location_rules', $create_locs);
				
				$required_fields = $asset->attr('required_fields');
				$required_fields = Array($old_create_type => $required_fields);
								
				$asset->setAttrValue('required_fields', $required_fields);
				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade [from 0.2] Failed', E_USER_WARNING);
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes'); 
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			$this->_upgrade03To04();
			bam('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.2');
		}//end version 0.2 upgrade

		if ($current_version == '0.3') {
			bam('STARTING ASSET BUILDER UPGRADE - VERSION 0.3');
			$this->_upgrade03To04();
			bam('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.3');
		}
		return true;

	}//end _upgrade()


	function _upgrade03To04()
	{
			// The create locations regime was radically overhauled between versions 0.3 and 0.4
			// to add more functions and more flexibility

			$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;


				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) { 
					trigger_error('Asset Builder Upgrade [to 0.4] Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					return false;
				}
	
				$old_link_type = $asset->attr('create_link_type');
				
				// convert the fixed create locations by adding the link types
				$old_create_locations = $asset->attr('create_location');
				if (!array($old_create_locations)) $old_create_locations = Array(0 => $old_create_locations);
				$fixed_create_locations = Array();
				foreach ($old_create_locations as $assetid => $null) {
					$fixed_create_locations[$assetid] = $old_link_type;
				}
				$asset->setAttrValue('create_location', $fixed_create_locations);

				// convert the selectable (optional) create locations by creating a set of the old ones
				if ($asset->attr('create_location_optional')) {
					$old_input_style = ($asset->attr('create_location_type') == 'select') ? 'dropdown' : 'radio';
					$old_selectable_locations = array_values($asset->attr('create_location_optional'));
					if (!is_array($old_selectable_locations)) $old_selectable_locations = Array($old_selectable_locations);
					$old_allow_multiple = $asset->attr('create_location_multiple') ? 1 : 0;
					$selectable_locations_set_details = Array();
					$location_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($old_selectable_locations);
					$selectable_locations_set_details['locations'] = Array();
					foreach ($old_selectable_locations as $assetid) {
						$selectable_locations_set_details['locations'][$assetid] = $location_details[$assetid]['name'];
					}
					echo "\n\n Allow Multiple is $old_allow_multiple\n\n";
					$selectable_locations_set_details['allow_multiple'] = $old_allow_multiple;
					$selectable_locations_set_details['allow_empty'] = 0;
					$selectable_locations_set_details['link_type'] = $old_link_type;
					$selectable_locations_set_details['input_style'] = $old_input_style;
					$asset->setAttrValue('create_location_optional', Array('First Selectable Location' =>  $selectable_locations_set_details));
				}

				// convert the create location rules into the new format
				$old_rules = $asset->attr('create_location_rules');
				$create_location_rules = Array();

				foreach ($old_rules as $create_type => $type_rules) {
					$create_location_rules[$create_type] = Array();
					foreach ($type_rules as $i => $rule_details) {
						$new_rule = Array('attributes' => Array(), 
										  'comparators' => Array(), 
										  'values' => Array(), 
										  'locations' => Array());
						foreach ($rule_details as $attr_name => $conditions) {
							if ($attr_name == '_create_locations_') continue;
							foreach ($conditions as $comparator => $value) {
								$new_rule['attributes'][] = $attr_name;
								$new_rule['comparators'][] = $comparator;
								$new_rule['values'][] = $value;
							}
						}
						if (is_array($rule_details['_create_locations_'])) {
							foreach ($rule_details['_create_locations_'] as $assetid) {
								$new_rule['locations'][] = $assetid;
								$new_rule['link_types'][] = $old_link_type;
							}
						} elseif ($rule_details['_create_locations_']) {
							$new_rule['locations'][$rule_details['_create_locations_']] = $old_link_type;
						}
						$create_location_rules[$create_type][] = $new_rule;
					}
				}
				$asset->setAttrValue('create_location_rules', $create_location_rules);

				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade [to 0.4] Failed - Couldn\'t save attributes', E_USER_WARNING);
					return false;
				}

				// change the %create_location% keyword in bodycopies to the keyword for our new set
				$content_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'content_type_wysiwyg');
				foreach (array_keys($content_assetids) as $assetid) {
					$content_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($content_asset->id, 'attributes')) { 
						trigger_error('Asset Builder Upgrade [to 0.4] Failed - Unable to Acquire Lock on "'.$content_asset->name.'" (#'.$content_asset->id.')', E_USER_WARNING);
						return false;
					}					
					$html = $content_asset->attr('html');
					$html = str_replace('%create_location%', '%first_selectable_location_field%', $html);
					$content_asset->setAttrValue('html', $html);
					$content_asset->saveAttributes();
					$content_asset->_updated();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($content_asset->id, 'attributes'); 
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_asset);
				}

				// regenerate the content files of the bodycopy containers so that they notice
				$container_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'bodycopy_container', false);
				foreach (array_keys($container_assetids) as $assetid) {
					$container_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($container_asset->id, 'attributes')) { 
						trigger_error('Asset Builder Upgrade [to 0.4] Failed - Unable to Acquire Lock on "'.$container_asset->name.'" (#'.$container_asset->id.')', E_USER_WARNING);
						return false;
					}					
					$container_asset->saveSystemVersion();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($container_asset->id, 'attributes'); 
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container_asset);
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes'); 
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade [to 0.4] Failed - Couldn\'t save attributes', E_USER_WARNING);
					return false;
				}

				$asset->_updated();
			}

			// get rid of the old attributes
			$this->_deleteAttribute('create_location_multiple');
			$this->_deleteAttribute('create_location_type');
			$this->_deleteAttribute('create_link_type');

	}//end _upgrade03To04()

}//end class

?>