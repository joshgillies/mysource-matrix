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
* $Id: page_asset_builder_management.inc,v 1.8 2004/07/28 01:47:19 lwright Exp $
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

		$this->vars['create_location']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		$this->vars['create_location_optional']	= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		$this->vars['create_location_rules']	= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		$this->vars['create_location_multiple']	= Array(
														'added'     => '0.1',
														'type'      => 'boolean',
														'default'   => false,
														);
		$this->vars['create_location_type']		= Array(
														'added'      => '0.1',
														'type'       => 'selection',
														'default'    => 'select',
														'parameters' => Array(
																			'options' => Array(
																					'select'	=> 'Dropdown List',
																					'check'		=> 'Checkboxes',
																			),
																		),
														);
		$this->vars['required_fields']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		$this->vars['create_type']				= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
													);
		$this->vars['create_link_type']			= Array(
														'added'     => '0.1',
														'type'      => 'int',
														'default'   => 1,
													);
		$this->vars['create_status']			= Array(
														'added'      => '0.1',
														'type'       => 'int',
														'default'    => 2,
													);


		$this->vars['login_invite']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Sign in using the form below.</p>',
													);
		$this->vars['login_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Incorrect username or password entered. Please try again.</p>',
													);
		$this->vars['create_invite']			= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Create an asset using the form below.</p>',
													);
		$this->vars['create_error']				= Array(
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
		if (!parent::_upgrade($current_version)) return false;

		if ($current_version == '0.1') {
			// this upgrade changed the attribute 'create_locations' to include additional
			// rules that meant the create locations has to be stored in a different format
			bam('STARTING ASSET BUILDER UPGRADE - VERSION 0.1');

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
				
				// 0.1->0.3 upgrades - see v0.2 comments below for more info 
				$create_type = $asset->attr('create_type');
				$create_type = Array($create_type => true);
								
				$asset->setAttrValue('create_type', $create_type);
				
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

				$old_create_type = $asset->attr('create_type');
				$create_type = Array($old_create_type => true);
								
				$asset->setAttrValue('create_type', $create_type);
				
				$create_locs = $asset->attr('create_location_rules');
				$create_locs = Array($create_type => $create_locs);
								
				$asset->setAttrValue('create_location_rules', $create_locs);
				
				$required_fields = $asset->attr('required_fields');
				$required_fields = Array($create_type => $required_fields);
								
				$asset->setAttrValue('required_fields', $required_fields);
				if (!$asset->saveAttributes()) { 
					trigger_error('Asset Builder Upgrade [from 0.2] Failed', E_USER_WARNING);
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes'); 
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			
			bam('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.2');
		}//end version 0.2 upgrade

		return true;

	}//end upgrade()


}//end class

?>
