<?php
/**
 * A DSN Pool object keeps a list of DSN objects.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author	   Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

require_once dirname(__FILE__).'/DSN.inc';

/**
 * DSNPool.
 *
 * A DSNPool object stores a list of DSN objects and provide methods for weighted-random
 * selection, failure registration, etc.
 *
 * @since 4.0.0
 */
class DSNPool
{


	/**
	 * The pool of DSN objects
	 *
	 * @var array
	 */
	private $_pool;

	/**
	 * The index of the current selected DSN.
	 * It has the default value of -1 which means that no DSN is selected yet.
	 *
	 * @var int
	 */
	private $_currentIndex = -1;


	/**
	 * Constructor
	 *
	 * @param array $dsn_arr	The array of DSNs
	 * @access public
	 */
	public function __construct(Array $dsn_arr)
	{
		$this->_pool = Array();
		foreach ($dsn_arr as $dsn) {
			$this->_pool[] = new DSN($dsn);
		}

	}//end constructor


	/**
	 * Register a DSN object as failure so that it will not be selected by the random selection.
	 * If no index is passed in, the current selected index will be used.
	 *
	 * @param int $index	The index of the DSN object to register as failure
	 * @return void
	 * @access public
	 */
	public function registerFailureDSN($index = NULL)
	{
		// If there is no index passed in, use the current index
		if (is_null($index)) {
			$index = $this->_currentIndex;
		}

		if (isset($this->_pool[$index])) {
			$this->_pool[$index]->setFailure(TRUE);
		}

	}//end registerFailureDSN()


	/**
	 * Get DSN array from a DSN object specified by the index.
	 *
	 * @param int $index	The index to get the DSN from
	 * @return NULL|array	Return NULL if the index is -1 or invalid; otherwise, return
	 *                   	the DSN array of the indexed DSN object
	 * @access public
	 */
	public function getDSNFromIndex($index = -1)
	{
		// It is unlikely to happen but we reset the current index if the index is invalid
		if (!isset($this->_pool[$index])) {
			$index = -1;
		}

		// Set current index
		$this->_currentIndex = $index;

		// If index = -1, we just want to reset the current index
		if ($index == -1) {
			return NULL;
		}

		return $this->_pool[$index]->getDSN();

	}//end getDSNFromIndex()


	/**
	 * Get a weighted random DSN array from the DSN pool
	 *
	 * @return NULL|array	Return a weighted random DSN array if there is still an
	 *                   	active (non-failure) DSN object; otherwise, return NULL
	 * @access public
	 */
	public function getDSN()
	{
		// Get the weight and index arrays of the active DSN objects
		$dsn_weights = $this->_getActiveDSNWeights();
		// Get a weighted random index of the weight array
		$weight_index = $this->_getWeightedRandomIndex($dsn_weights['weights']);
		// Get the pool index from the active weight index
		$index = $weight_index == -1 ? -1 : $dsn_weights['indexes'][$weight_index];

		return $this->getDSNFromIndex($index);

	}//end getDSN()


	/**
	 * Get the weight and index arrays of the DSN objects which are not marked as failure.
	 *
	 * @return array	Return an array which contains both weight and index arrays of the DSN objects
	 * @access private
	 */
	private function _getActiveDSNWeights()
	{
		$weights = Array();
		$indexes = Array();
		foreach ($this->_pool as $index => $dsn) {
			if ($dsn->getFailure() !== TRUE) {
				$weights[] = $dsn->getWeight();
				$indexes[] = $index;
			}
		}

		return Array('weights' => $weights, 'indexes' => $indexes);

	}//end _getActiveDSNWeights()


	/**
	 * Get the random weighted index of a weight array. The weight array is just an array of integers, e.g. Array(10, 5, 20).
	 * This method will return a random index in each invocation but the bigger the weight the higher chances of that index
	 * will be returned. In the above example array, the index 2 is twice more likely to be returned than the index 0 which
	 * in turn is twice more likely to be returned than 1.
	 *
	 * @param array $weights	The weight array to get weighted random index
	 * @return int				Return -1 if the weight array is empty; Return 0 if the array only has one element; And return
	 *            				a random index based on the weights if there are 2 or more elements in the array
	 * @access private
	 */
	private function _getWeightedRandomIndex($weights)
	{
		switch (count($weights)) {
			case 0:
				// The weight array is empty, return -1 to indicate the invalid index
				return -1;
			case 1:
				// The weight array has one element, return the first index
				return 0;
		}

		// Get a random index number based on the weights. The bigger the weight of an array element, the bigger chance its index is selected.
		$total_weights = array_sum($weights);
		$random_number = mt_rand(1, $total_weights);
		$n = 0;
		foreach ($weights as $index => $weight) {
			$n += $weight;
			if ($n >= $random_number) {
				break;
			}
		}

		return $index;

	}//end _getWeightedRandomIndex()


}//end class


?>
