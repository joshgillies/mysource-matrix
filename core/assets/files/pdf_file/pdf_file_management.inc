<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: pdf_file_management.inc,v 1.6 2006/12/05 23:14:14 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
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

	}//end constructor


}//end class
?>
