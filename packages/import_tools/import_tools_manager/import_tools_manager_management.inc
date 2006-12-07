<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright © Squiz Pty Ltd      |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: import_tools_manager_management.inc,v 1.4 2006/12/07 00:25:40 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Import_Tools_Manager_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tools_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Import_Tools_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'import_dir'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'valid_file_types'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'htm',
																	'html',
																   ),
												   ),
						'valid_word_file_types'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'html',
																	'htm',
																   ),
												   ),
						'heading_defaults'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'h1',
																	'h2',
																   ),
												   ),
						'max_file_size'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'int',
													'default'	=> 0,
												   ),
						'create_css_file'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'allowed_import_type'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'Server and Local',
													'is_admin'		=> 0,
													'parameters'	=> Array(
																		'options'		=> Array(
																							'Server and Local'	=> 'Server and Local',
																							'Server Only'		=> 'Server Only',
																							'Local Only'		=> 'Local Only',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'allow_other'	=> FALSE,
																	   ),
												   ),
						'bulk_file_import_dir'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
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
		// Bump this version when a new converter is installed.
		// No need to have a new version compare statement, as any call to installConverters() below
		// will install all converters currently in the system
		if (version_compare($current_version,'0.2','<')) {

			echo('INSTALLING NEW CONVERTERS - FROM VERSION '.$current_version."\n");

			$itm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
			$itm->installConverters();

			echo('FINISHED INSTALLING NEW CONVERTERS - FROM VERSION '.$current_version."\n");

		}

		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class
?>
