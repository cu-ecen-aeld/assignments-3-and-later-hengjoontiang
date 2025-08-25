workings

mutex access to aesd file -- required due to timer signal vs network data write -- DONE


mutex access to arraylist -- recv thread write to arraylist
			     each string added to arraylist
			     for each string
			         append string to long_string
			     write_file(long_string)
			     
eventual separate mutex files






