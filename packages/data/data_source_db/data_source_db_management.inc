<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_db_management.inc,v 1.12 2007/12/22 01:47:19 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_DB_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_DB_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_DB_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'database'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'host'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'user'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'password'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'password',
											'default'	=> '',
										   ),
						'db_type'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'postgres',
										   ),
						'sql'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'where'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'group_by'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'order_by'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'having'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'tables'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'fields'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'custom_fields'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'aggregates'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'join'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'mode'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'parameters'	=> Array (
																'options'		=> Array(
																					'builder'	=> 'Query Builder',
																					'edit'		=> 'Edit Manually',
																					'execute'	=> 'Execute',
																				   ),
																'multiple'		=> FALSE,
																'allow_other'	=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'default'		=> 'builder',
										   ),
						'edit_on'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										   ),
						'reset_edit'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										   ),
						'limit'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '',
											'parameters'	=> Array(
																'allow_empty'		=> TRUE,
																'allow_negative'	=> FALSE,
															   ),
										   ),
						'dynamic_vars'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'parameter_map'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'parameter_map',
											'default'		=> Array(),
											'parameters'	=> Array(
																'parameters'	=> Array(),
															   ),
										   ),
					  );

	}//end constructor


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset should be upgraded to
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// Data_Source_Db does not store '<br />' string in 'sql' attribute anymore
		if (version_compare($current_version,'0.3','<')) {

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('data_source_db', FALSE);

			pre_echo('UPGRADING DATA SOURCE DB - FROM VERSION '.$current_version);

			$upgraded_count = 0;
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$sql = $asset->attr('sql');
				if (!$asset->setAttrValue('sql', str_replace('<br />', ' ', $sql))) {
					trigger_localised_error('CMS0039', E_USER_WARNING, '0.3');
				}
				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0039', E_USER_WARNING, '0.3');
				}
				$upgraded_count++;
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			pre_echo('Upgraded '.$upgraded_count.' DATA SOURCE DB assets to version 0.3');

		}//end if version 0.3

		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class

?>
