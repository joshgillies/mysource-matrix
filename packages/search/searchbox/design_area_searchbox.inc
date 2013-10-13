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
* $Id: design_area_searchbox.inc,v 1.35.8.1 2013/10/13 23:00:49 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Design_Area_SearchBox
*
* Searchbox - A design area that shows a search box for 'quick'-searching.
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.35.8.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Design_Area_Searchbox extends Design_Area
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
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @return boolean
	* @access public
	* @see Design_Area::printArea()
	*/
	function _canInit()
	{
		$assetid = $this->attr('search_page_assetid');
		return !($assetid <= 0);

	}//end _canInit()


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		$assetid = $this->attr('search_page_assetid');
		if (empty($assetid)) return;
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset = $am->getAsset($assetid);

		switch ($var) {

			case 'form_start' :
				echo '
					<?php $asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.'); ?>
					<form method="get" action="<?php echo $asset->getURL(); ?>">
				';
			break;

			case 'form_end' :
				echo '</form>';
			break;

			case 'submit' :
				if ($this->attr('submit_button')) {
					$class = isset($args['class']) ? ' class="'.$args['class'].'"' : '';

					$button_value = $this->attr('submit_button_text');
					echo '<input type="submit" value="'.$button_value.'"'.$class.' />';

				} else {
					$submit_image_assetid = $this->attr('submit_image_assetid');
					if ($submit_image_assetid > 0) {
						$submit_image_asset = $am->getAsset($submit_image_assetid);
						echo '<'.'?php $submit_image_asset = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$submit_image_asset->id.', "'.$submit_image_asset->type().'"); ?'.'>';
						echo '<input alt="'.$this->attr('submit_button_text').'" type="image"';
						if (isset($args['class'])) {
							echo ' class="'.$args['class'].'"';
						}
						echo ' src="<'.'?php echo $submit_image_asset->getURL(); ?'.'>" />';
					} else {
						// no image defined
					}
				}
			break;

			case 'category_list':
				$category_set = $this->attr('category_set');
				$default = $this->attr('category_default');
				$show_empty = $this->attr('category_show_empty');
				if (!empty($category_set)) {
					echo '
						<?php
							$asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.');
							$categories = $asset->attr("categories");
							echo $asset->getCategoryChooserReplacement("'.$category_set.'", $categories["'.$category_set.'"], "'.$default.'", '.var_export($show_empty, TRUE).');
						?>';
				}

			break;

			case 'box' :
				$class = isset($args['class']) ? ' class="'.$args['class'].'"' : '';
				$id = isset($args['id']) ? ' id="'.$args['id'].'"' : '';

				$title_val = $this->attr('querybox_title');
				$title = !empty($title_val) ? ' title="'.$this->attr('querybox_title').'"' : '';

				$label_val = $this->attr('querybox_label');
				$label = '';

				if (!empty($label_val)) {
					if (!empty($args['id'])) {
						$form_field_name = $args['id'];
					} else {
						$form_field_name = htmlspecialchars($this->attr('field_name'));
						$form_field_name = 'queries_'.$form_field_name.'_query';
					}

					$label = '<label for="'.$form_field_name.'">'.$this->attr('querybox_label').'</label>';
				}
				echo '
				<input type="hidden" name="mode" value="results" />
				<?php
					$asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.');

					$field_name = "'.$this->attr('field_name').'";
					$box_width  = '.$this->attr('querybox_width').';
					if (!empty($field_name)) {
						$form_field_name = "queries_".$field_name."_query";
						$field_value     = $asset->getQueryFieldValue($field_name);

						if (empty($field_value)) {
							$field_value = "'.$this->attr('default_text').'";
							$hide_default = true;
						} else {
							$hide_default = false;
						}

						require_once SQ_LIB_PATH."/html_form/html_form.inc";
						'.(!empty($label) ? 'echo \''.$label.'\';' : '').'
						text_box($form_field_name, $field_value, $box_width, "", $hide_default, \''.$class.$id.$title.'\');
					}
				?>
				';
			break;

			default:
				if (substr($var, -6) == '_query') {
					echo '
					<?php
						$asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.');
						echo $asset->getFieldReplacement("'.substr($var, 0, -6).'");
					?>';
				} else if (substr($var, -17) == '_category_chooser') {
					echo '
					<?php
						$asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.');
						$categories = $asset->attr("categories");
						echo $asset->getCategoryChooserReplacement("'.substr($var, 0, -17).'", $categories["'.substr($var, 0, -17).'"]);
					?>';
				}
			break;

		}//end switch

	}//end printVar()


}//end class

?>
