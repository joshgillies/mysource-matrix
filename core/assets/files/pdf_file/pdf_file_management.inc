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
* $Id: pdf_file_management.inc,v 1.4 2005/05/04 00:38:04 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class PDF_File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function PDF_File_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'password'	=> Array(
										'added'			=> '0.2',
										'type'			=> 'password',
										'default'		=> '',
										'description'	=> 'Password-protected PDFs cannot be indexed without the password',
									   ),
					  );

		$this->components = Array(
								'contents'	=> Array(
												'description'		=> 'Contents (text format)',
												'default_weighting'	=> '1',
											   ),
							);

	}//end constructor


}//end class
?>
