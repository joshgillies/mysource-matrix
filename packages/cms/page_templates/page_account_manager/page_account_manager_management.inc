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
* $Id: page_account_manager_management.inc,v 1.9 2004/07/28 01:41:54 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Account_Manager_Management
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Account_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['create_type']				= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array('user' => 1),
													);

		$this->vars['login_invite']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>If you already have an account, sign in using the form below.</p>',
													);
		$this->vars['login_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Incorrect username or password entered. Please try again.</p>',
													);
		$this->vars['create_invite']			= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Sign up for an account using the form below.</p>',
													);
		$this->vars['create_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Your account was not created. Ensure you have entered a username and password and try again.</p>',
													);
		$this->vars['edit_details_invite']		= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Edit your details using the form below.</p>',
													);
		$this->vars['edit_details_lock_error']	= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Failed to acquire lock for editing. Your account details are displayed below.</p>',
													);
		$this->vars['edit_details_success']		= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Your account details have been updated.</p>',
													);
													
		$this->vars['use_email_validation']		= Array(
														'added'     => '0.1',
														'type'      => 'boolean',
														'default'   => false,
													);

		$this->vars['validation_email_format']	= Array(
														'added'     => '0.1',
														'type'      => 'email_format',
														'default'   => Array(),
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
			// this update is due to the asset builder changes in that asset's v0.3 to handle
			// the creation of multiple asset types - although account manager can still only
			// create one type of user, the underlying structure has changed so we need this
			bam('STARTING ACCOUNT MANAGER UPGRADE - VERSION 0.1');

			$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) { 
					trigger_error('Account Manager Upgrade [from 0.1] Failed - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					return false;
				}

				$create_type = $asset->attr('create_type');
				$create_type = Array($create_type => true);
								
				$asset->setAttrValue('create_type', $create_type);
				if (!$asset->saveAttributes()) { 
					trigger_error('Account Manager Upgrade [from 0.1] Failed', E_USER_WARNING);
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes'); 
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			
			bam('ACCOUNT MANAGER UPGRADE COMPLETE - VERSION 0.1');
		}//end version 0.1 upgrade

		return true;

	}//end _upgrade()
	
	
}//end class

?>