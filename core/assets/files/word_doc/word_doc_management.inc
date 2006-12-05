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
* $Id: word_doc_management.inc,v 1.5 2006/12/05 23:14:15 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Word_Doc_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Word_Doc_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->components = Array(
								'contents'	=> Array(
												'description'		=> 'Contents (text format)',
												'default_weighting'	=> '1',
											   ),
							);

	}//end constructor


}//end class
?>
