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
* $Id: metadata_field_thesaurus_management.inc,v 1.1 2004/11/30 01:29:10 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Metadata_Field_Thesaurus_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Metadata_Field_Thesaurus_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// additional attrs., that were not defined in parent
		$this->vars = Array(
			'thesaurus_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'assetid',
				'default'		=> '',
				'description'   => 'Thesaurus asset which will supply the data to this metadata field.',
			),

			'allow_manual' => Array(
				'added'       => '0.0.1',
				'type'        => 'boolean',
				'default'     => false,
				'description' => 'Whether to allow for manual input of this field',
				'parameters' => Array(
						'allow_empty' => false
					),
			),

			'force_thesaurus' => Array(
				'added'       => '0.0.1',
				'type'        => 'boolean',
				'default'   => false,
				'description' => 'If set to true will only accept values from the thesaurus, to be used with allow_manual',
				'parameters' => Array(
						'allow_empty' => false
					),
			),

		);

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
			'files/browser.html',
			'files/thesaurus_browser.js',
			'files/images/1x1.gif',
			'files/images/arrowl.gif',
			'files/images/arrowr.gif',
			'files/images/minus.gif',
			'files/images/plus.gif',
			'files/images/thesaurus_button.gif',
			'files/images/bs.png',
			'files/images/lbs.png',
			'files/images/ls.png',
			'files/images/lts.png',
			'files/images/rbs.png',
			'files/images/rs.png',
			'files/images/rts.png',
			'files/images/rts2.png',
			'files/images/ts.png',
		);

	}//end _getFileList()

}//end class

?>