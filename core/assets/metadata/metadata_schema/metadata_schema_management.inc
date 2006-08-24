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
* $Id: metadata_schema_management.inc,v 1.11.4.1 2006/08/24 23:54:14 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Schema_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11.4.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Schema_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'namespace'	=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'nsurl'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'frontend'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'parameters'	=> Array('allow_empty'	=> FALSE),
									   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currently installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			// metadata schema names, sections, fields, etc. are now escaped
			// for safety's sake, regenerate all the schemas in the system
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.2');
			// get all 'metadata_schema' assets (or their descendants)
			$schemas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE);
			pre_echo('Upgrading '.count($schemas).' assets...');

			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaids' => $schemas);
			$hh->freestyleHipo('hipo_job_regenerate_metadata', $vars);

			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.2');

		}//end version 0.2 upgrade

		return TRUE;

	}//end _upgrade()


}//end class

?>