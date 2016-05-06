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
* $Id: content_type_raw_html.inc,v 1.7 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Raw_HTML
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML extends Content_Type
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Check that HTML content has the matching opening and closing div tags whening changing the content type to WYSIWYG
	* HTML content with non-matchting div tags will conflict with Viper functionalities
	*
	* @param string $new_content_type
	*
	* @return boolean
	* @access public
	*/
	public function validateContent($new_content_type)
	{
		// Only relevant when changing content type to WYSIWYG
		if ($new_content_type != 'content_type_wysiwyg') {
			return TRUE;
		}

		$content = $this->attr('html');
		preg_match_all('/<div[^>]*>/msi', $content, $matches);
		$opening_div_tags = count($matches[0]);
		preg_match_all('/<\/div>/msi', $content, $matches);
		$closing_div_tags = count($matches[0]);

		if ($opening_div_tags != $closing_div_tags) {
			trigger_localised_error('CMS0123', translate('Cannot change content type to WYSIWYG. The HTML content must contain matching opening and closing "div" tags.'), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

    }//end validateContent()


}//end class

?>
