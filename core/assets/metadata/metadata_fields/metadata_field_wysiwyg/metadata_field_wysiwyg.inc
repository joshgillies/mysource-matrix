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
* $Id: metadata_field_wysiwyg.inc,v 1.2 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata_Field_WYSIWYG
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_WYSIWYG extends Metadata_Field
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
	* Output the JS code to untick the "Use Default" checkbox
	* after populating the respective metadata field
	*
	* @param string $el_id	Id of the metadata field's "use default" checkbox element
	*
	* @return void
	* @access protected
	* @see Metadata Field::printRepopulateFieldJs()
	*/
	protected function _printUntickUseDefaultCheckboxJs($el_id)
	{
		if (!isset($_POST[$el_id])) {
			?>
			if (document.getElementById('<?php echo htmlspecialchars($el_id, ENT_QUOTES); ?>')) {
				document.getElementById('<?php echo htmlspecialchars($el_id, ENT_QUOTES); ?>').click();
			}
			<?php
		}

	}//end _printUntickUseDefaultCheckboxJs()


}//end class

?>
