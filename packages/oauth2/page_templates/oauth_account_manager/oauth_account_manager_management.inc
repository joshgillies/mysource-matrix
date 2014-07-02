<?php
/**
* +-----------------------------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd           |
* | ABN 77 084 670 600                                                                            |
* +-----------------------------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find            |
* | this file please contact Squiz (www.squiz.com.au) so we may provide |
* | you a copy.                                                                                         |
* +-----------------------------------------------------------------------------------------+
*
* $Id: oauth_account_manager_management.inc,v 1.2 2013/07/19 05:27:41 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Oauth_Account_Manager_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth_Account_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars['create_type']				= Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array('user' => TRUE),
												  );
		
		$this->vars['redirect_url']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'URL to redirect to after successful Oauth authentication',
								
									);
		$this->vars['use_querystring_parameter']			= Array(
										'added'			=> '0.2',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Query string parameter name whose value is to be used as the redirect destination',
									    );
		$this->vars['redirect_after_login']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['redirect_after_create']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['redirect_after_link']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['allow_link']					= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['auto_create']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );
		$this->vars['userid_api_url']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'Rest API URL to get user id',
								
									);
		$this->vars['userid_location']				= Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'description'	=> 'JSON element of user id in API response',
								
									);
		$this->vars['prefilled_fields']				= Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );
		$this->vars['synced_fields']				= Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );		
		$this->vars['randomize_password']				= Array(
										'added'		=> '0.1',
										'type'		=> 'boolean',
										'default'		=> TRUE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    );

	}//end constructor

	
	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
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
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING Oauth Account Manager TO v0.2');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('oauth_account_manager');
			foreach ($assetid_list as $assetid) {
				$oauth_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				$prefilled_fields = $oauth_asset->attr('prefilled_fields');
				if (!empty($prefilled_fields)) {
					foreach ($prefilled_fields as $create_type => $data) {
						foreach ($data as $attr => $value) {
							if(strpos($value, '%') === FALSE) {
								$new_value = implode('_', explode('.', $value));
								$new_value = '%globals_session_oauth_profile_'.$new_value.'%';
								$prefilled_fields[$create_type][$attr] = $new_value;
							}
						}
					}

					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$oauth_asset->setAttrValue('prefilled_fields', $prefilled_fields);
					if (!$oauth_asset->saveAttributes()) {
						pre_echo('UPGRADE FAILED - Could not save attributes');
						return FALSE;
					}//end if
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($oauth_asset);
			}
			pre_echo(count($assetid_list).' Oauth Account Manager asset(s) UPDATED');
		}

		return $res;
	}

}//end class
?>
