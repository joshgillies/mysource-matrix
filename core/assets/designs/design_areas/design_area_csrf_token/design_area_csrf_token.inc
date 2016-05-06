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
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Csrf_Token
*
* Prints the CSRF Token on the page.
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.39 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Csrf_Token extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Print the 
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		echo '<','?php';
		// NB we are leaving PHP mode here so that
		// the following code is echoed rather than executed
		?>

        include_once SQ_FUDGE_PATH.'/general/general.inc';
        include_once SQ_LIB_PATH.'/html_form/html_form.inc';
        echo hidden_field('token', get_unique_token());

		<?php
		echo '?','>';

		return TRUE;

	}//end _printPaintingCode()


	/**
	* Returns true if this content generated by this design area can be cached
	*
	* @return boolean
	* @access public
	*/
	public function canBeCached()
	{
		return FALSE;

	}//end canBeCached()


}//end class
?>