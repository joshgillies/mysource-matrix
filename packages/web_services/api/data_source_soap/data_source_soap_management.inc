<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_soap_management.inc,v 1.6 2013/09/10 03:15:04 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Soap_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Data_Source_Soap_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Soap_Management(&$pm)
	{
		$this->Asset_Management($pm);
		
		require_once SQ_PACKAGES_PATH.'/web_services/api/lib/soap_common.inc'; 
		$auth_types = Soap_Common::$auth_types;

		$this->vars	= Array (
						'name'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The name of the datasource',
											   ),
						'wsdl'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'SOAP service WSDL link',
											   ),
						'username'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Authentication login username',
											   ),
						'password'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'password',
												'default'		=> '',
												'description'	=> 'Authentication password',
											   ),
						'auth_type'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'HTTP_BASIC',
												'parameters'    => Array(
																		'multiple'      => FALSE,
																		'allow_empty'   => FALSE,
																		'options'       => $auth_types,
																	),
												'description' 	=> 'Authentication type to use',
											   ),
						'local_cert'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description' 	=> 'Asset ID of the file asset for the HTTPS client certificate authentication',
											   ),
						'passphrase'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
												'description' 	=> 'Passphrase for the HTTPS client certificate authentication, if applicable',
											   ),
						'mute_exception'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'boolean',
												'default'	=> FALSE,
												'parameters'    => Array( 'allow_empty' => FALSE,),
												'description' 	=> 'Whether to mute SOAP expection in response',
											   ),
						'request_fn'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
												'description' 	=> 'Selected SOAP request function',
											   ),
						'soap_requests'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'description' 	=> 'Available SOAP requests',
											   ),
						'file_keywords'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Data record set keywords for SOAP responses that are expected as a file',
											   ),
						'root_tag'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Root tag in XML response to look into',
											   ),
						'selected_tags'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Comma-separated list of tag name to capture',
											   ),
						'data_delimiter'	=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> ',',
												'description'	=> 'Delimiler to seperate data from xml elements with same name',
											   ),
		    			'jsessionid_url'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'JSESSION ID Authentication URL',
											   ),
    		    		'service_address'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Selected Service Address to send request to',
											   ),
						'auth_details'		=> Array(
												'added'		=> '0.2',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'description' 	=> 'SOAP authentication details',
											   ),
					  );

	}//end constructor
	
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
			// Auth types connection details are stored in serialised "auth_details" attr now
			// Also make the "USERNAME_TOKEN" and "WSS_DIGEST" auth type value consistant with the Soap_Common,
			// which now used for painting/processing auth type fields
			// Regarding major code refactoring changes when adding feature #6584
			pre_echo('Upgrading SOAP Data Soruce to v0.2');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('data_source_soap', FALSE);
			foreach ($assetid_list as $assetid) {
				$soap_ds = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$username = $soap_ds->attr('username');
				$password = $soap_ds->attr('password');
				$login_url = $soap_ds->attr('jsessionid_url');
				$auth_type = $soap_ds->attr('auth_type');
				
				$auth_settings = Array();
				$upgrade_required = FALSE;
				if (!empty($username) || !empty($password) || !empty($login_url)) {
					$auth_settings['login'] = $username;
					$auth_settings['password'] = $password;
					$auth_settings['login_url'] = $login_url;
					$upgrade_required = TRUE;					
				}
				
				// No upgrade requires for these cases
				if ($auth_type == 'USERNAME_TOKEN' ) {
					$auth_type = 'WSS_USERNAME_TOKEN';
					$upgrade_required = TRUE;
				}
				
				if ($auth_type == 'WSS_DIGEST') {
					$auth_type = 'WSS_PASSWORD_DIGEST';
					$upgrade_required = TRUE;
				}
				
				if (!$upgrade_required) {
					continue;
				}

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);					
					$soap_ds->setAttrValue('auth_type', $auth_type);
					$soap_ds->setAttrValue('auth_details', $auth_settings);					
					if (!$soap_ds->saveAttributes()) {
						pre_echo('UPGRADE FAILED - Could not save attributes for asset #'.$soap_ds->id);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($soap_ds, TRUE);
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}//end if
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($soap_ds, TRUE);
			}
			pre_echo(count($assetid_list).' Default Delivery Method updated');
		}

		return $res;

	}//end _upgrade()	


}//end class
?>
