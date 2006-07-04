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
* $Id: metadata_field_management.inc,v 1.18.2.2 2006/07/04 02:42:33 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'friendly_name'		=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'description'		=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'default'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'scheme'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'language'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'value_components'	=> Array(
												'added'	=> '0.0.1',
												'type'	=> 'serialise', 'default'	=> Array(),
											   ),
						'editable'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'required'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
												'description'	=> 'If this field is Required, then either the value or any one of the value components must be set for an asset to be approved',
											   ),
						'frontend'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'show',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'show'	=> 'Always',
																						'fill'	=> 'Only If Filled',
																						'hide'	=> 'Never',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
					  );

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'popups/keywords.php',
				'popups/keyword_extraction.php',
			   );

	}//end _getFileList()


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

		if (version_compare($current_version,'0.2','<')) {
			// changes metadata_field_* is_exclusive value to 0 so that they can be moved
			// to other metadata sections

			pre_echo('UPGRADING METADATA FIELD - FROM VERSION '.$current_version.' TO VERSION 0.2');

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			// get number of assets to upgrade
			$sql = 'SELECT count(assetid) FROM sq_ast WHERE type_code LIKE '.$db->quote('metadata_field_%');
			$result = $db->getOne($sql);
			assert_valid_db_result($result);

			pre_echo('Upgrading '.$result.' assets');

			$sql = 'UPDATE sq_ast_lnk SET is_exclusive = '.$db->quote('0').'
					WHERE linkid IN (SELECT linkid FROM sq_ast a
					LEFT OUTER JOIN sq_ast_lnk l ON a.assetid = l.minorid
					WHERE a.type_code LIKE '.$db->quote('metadata_field_%').'
						AND l.is_exclusive = '.$db->quote('1').'
						AND l.is_dependant = '.$db->quote('1').')';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if

		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class

?>
