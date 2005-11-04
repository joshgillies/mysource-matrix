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
* $Id: metadata_field_thesaurus_management.inc,v 1.5 2005/11/04 06:02:04 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
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

		$this->vars = Array(
						'thesaurus_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Thesaurus asset which will supply the data to this metadata field.',
											   ),
						'allow_manual'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'description'	=> 'Whether to allow for manual input of this field',
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
											   ),
						'force_thesaurus'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'description'	=> 'If set to true will only accept values from the thesaurus, to be used with allow_manual',
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
											   ),
						'max_terms'			=> Array(
												'added'			=> '0.3',
												'type'			=> 'int',
												'default'		=> '1',
												'description'	=> 'The maximum number of terms to be specify.',
												'parameters'	=> Array(
																	'allow_negative'	=> false,
																   ),
											   ),
					  );

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/browser.html',
				'files/thesaurus_browser.js',
				'files/images/info_on.png',
				'files/images/info_off.png',
				'files/images/cross.png',
				'files/images/minus.gif',
				'files/images/plus.gif',
				'files/images/thesaurus_button.gif',
			   );

	}//end _getFileList()


}//end class

?>