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
* $Id: product.inc,v 1.1 2005/01/12 23:57:01 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Product
* 
* product class description here
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
 
class Product extends Asset
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function Product($assetid=0)
	{
		$this->Asset($assetid);
		
	}//end Product()
	
	
	function printBody()
	{
		/* Frontend for a product will be drawn using a keyword replacements
		 * type structure. Since the contents are generally static, the 
		 * keyword replacements list can be directly associated with the assets
		 * attributes. 
		 * NOTE: if images are linked, how to do that with keyword
		 * replacements?
		 */
		print('blah');
		
		//keywords array for keywords replacements
		$keywords = array(
						  'description'			=> $this->attr('description'),
						  'short_description'   => $this->attr('short_description'),
						  'details'				=> $this->attr('details'),
						  'name'				=> $this->name('name'),
						  'price'				=> $this->price('price')
						  );
		
		//prepare display output
		$format = ''; //get format from user, add attribute for wysiwyg
		echo replace_keywords($format, $keywords);
		
	}//end printBody()
	
	
}//end class
?>
