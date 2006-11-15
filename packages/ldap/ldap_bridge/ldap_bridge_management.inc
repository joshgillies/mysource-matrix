<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ldap_bridge_management.inc,v 1.11 2006/11/15 23:50:01 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* LDAP_Bridge_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function LDAP_Bridge_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'host'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'port'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '389',
											   ),
						'root_dn'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'bind_dn'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'password'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
											   ),
						'system_type'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'openldap',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'openldap'			=> 'OpenLDAP',
																						'activedirectory'	=> 'Active Directory',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'ldap_options'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'attributes'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'user_attributes'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'user_type'			=> Array(
												'added'			=> '0.4',
												'type'			=> 'selection',
												'default'		=> 'ldap_backend_user',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'ldap_backend_user'	=> 'Backend User',
																						'ldap_user'			=> 'User',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
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
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.3', '<')) {
			// version 0.2 -> 0.3
			// removed the openldap3 option from the 'system_type' attribute
			// and made the use of protocol version 3 an option in the
			// 'ldap_options' attribute
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.3');
			$pl_ldap_bridges = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			pre_echo('Updating '.count($pl_ldap_bridges).' asset(s)...');
			foreach ($pl_ldap_bridges as $assetid => $type_code) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				// We need to output our content files, otherwise old paint layouts will print blank
				if ($asset->attr('system_type') == 'openldap3') {
					$asset->setAttrValue('system_type', 'openldap');
					$asset->setAttrValue('ldap_options', Array('LDAP_OPT_PROTOCOL_VERSION' => 3));

					$asset->saveAttributes();
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.3');

		}//end version 0.3 upgrade
		return TRUE;

	}//end _upgrade()


}//end class

?>