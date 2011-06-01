<?php
require_once SQ_PACKAGES_PATH.'/ecommerce/delivery_methods/ecommerce_delivery_method/ecommerce_delivery_method_edit_fns.inc';

class Default_Delivery_Method_Edit_Fns extends Ecommerce_Delivery_Method_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Default_Delivery_Method_Edit_Fns()
	{
		$this->Ecommerce_Delivery_Method_Edit_Fns();

	}//end constructor


	/**
	* Paints the available keywords
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAvailableKeywords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$tmp_form =& $asset->getForm();
			$availableKeywords = $tmp_form->getQuestions();

			// print the response keyword
			echo '<style type="text/css">#keyword_table td {border:1px solid #B3B3B3;}  #keyword_table {border:1px solid #000}</style>';
			echo '<table id="keyword_table"><tr><td><strong>Keyword</strong></td><td><strong>Description</strong></td></tr>';
			foreach ($availableKeywords as $key => $value) {
				$key_parts = split(':', $key);
				echo '<tr><td>%response_'.$key_parts[1].'%</td><td>print field value <strong>"'.$value['name'].'"</strong> </td></tr>';
			}
			echo '<tr><td>%customised_receipt%</td><td>print the customised receipt (if enabled via the checkout asset)</td></tr>';
			echo '<tr><td>%order_receipt%</td><td>print the order receipt</td></tr>';
			echo '<tr><td>%cart_summary%</td><td>print the cart summary</td></tr>';
			echo '<tr><td>%cart_summary_text%</td><td>print the cart summary in a text format</td></tr>';
			echo '<tr><td>%form_summary%</td><td>print the form summary</td></tr>';
			echo '<tr><td>%form_summary_text%</td><td>print the form summary in a text format</td></tr>';
			echo '<tr><td>%order_id%</td><td>print the order id</td></tr>';
			echo '<tr><td>%negative_quantity_product_list%</td><td>print products which have negative quantities after purchasing</td></tr>';
			echo '<tr><td>%negative_quantity_message%</td><td>print negative quantity products message if there are any negative quantities</td></tr>';
			echo '</table>';

		}

		return $write_access;

	}//end paintAvailableKeywords()


	/**
	* Dummy function
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAvailableKeywords(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processAvailableKeywords()


}//end class

?>
