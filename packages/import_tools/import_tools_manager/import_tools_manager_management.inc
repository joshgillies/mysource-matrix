<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: import_tools_manager_management.inc,v 1.9 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Import_Tools_Manager_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.9 $ - 0.1
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
						'cms_file_export_dir'	=> Array(
													'added'		=> '0.2',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'cms_file_export_name'	=> Array(
													'added'		=> '0.2',
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

			$itm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
			$itm->installConverters();

			echo('FINISHED INSTALLING NEW CONVERTERS - FROM VERSION '.$current_version."\n");

		}

		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class
?>
