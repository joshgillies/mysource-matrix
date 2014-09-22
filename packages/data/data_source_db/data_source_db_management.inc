<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_db_management.inc,v 1.15 2012/08/30 00:58:15 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_DB_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
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
					trigger_localised_error('CMS0039', sprintf(translate('Listing Engine Upgrade [from %s] FAILED'), '0.3'), E_USER_WARNING)
				}
				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0039', sprintf(translate('Listing Engine Upgrade [from %s] FAILED'), '0.3'), E_USER_WARNING)
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
