
#include "main.ih"

int main(int argc, char **argv)
{
	gettimeofday(&tim2, NULL);

	MPI::Init(argc, argv);
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);

	size_t rank = 0;
	size_t size = 1;
	try
	{
		rank = MPI::COMM_WORLD.Get_rank();
		size = MPI::COMM_WORLD.Get_size();
	}
	catch (MPI::Exception const &exception)
	{
		cerr << "MPI error: " << exception.Get_error_code() << " - "
				<< exception.Get_error_string() << endl;
	}


	if (argc == 3 && string(argv[1]) == "-d")
	{
		size_t n = 2;
		stringstream ss(argv[2]);
		ss >> n;

		double start = MPI::Wtime();

		// findFunction makes it very slow, this can be solved by replacing it
		// with the following, replacing N for the Dedekind number to compute.
		//Dedekind::UInt128* result = Dedekind::monotoneSubsets<2>(rank, size);
		Dedekind::UInt128* result = findFunction(n)(rank, size);

		double end = MPI::Wtime();
		cerr << "Rank " << rank << " done!" /* Result: " << result*/ << " in "
			  << end - start << "s\n";
		size_t length = n+1;
		// reduce over all processes
		if (rank == 0)
		{
			size_t toReceive = (size-1)*length+1;
			while (--toReceive)
			{
				// send the high and the low part of the result
				uint_fast64_t lohi[3];
				for( size_t i=0;i<3;i++){
					lohi[i]=0;
				}
				MPI::Status status;
				MPI::COMM_WORLD.Recv(lohi, 3, MPI::UNSIGNED_LONG,
						MPI::ANY_SOURCE, Dedekind::BIGINTTAG, status);

				Dedekind::UInt128 tmp(lohi[0], lohi[1]);
				result[lohi[2]] += tmp;
			}

			double final = MPI::Wtime();
			Dedekind::UInt128 dedekindnumber=0;
			Dedekind::UInt128 othernumber =0;			
			for(size_t i=0;i<=n;i++)
			{
				cout << result[i] << "\n";
				dedekindnumber += result[i];
				othernumber += result[i] *(1<<(n-i));
			}
			othernumber -= (1<<n);
			cout << "d" << (1<<n) << " = " << dedekindnumber << " and zeta" << n << " = " << othernumber
				<< " (in " << final - start << "s)\n";
		}
		else
		{
			// send the high and the low part of the result
			uint_fast64_t lohi[3];
			for(size_t i=0;i<length;i++){
				lohi[0] = result[i].lo();
				lohi[1] = result[i].hi();
				lohi[2] = i;
				MPI::COMM_WORLD.Bsend(&lohi, 3, MPI::UNSIGNED_LONG, 0,
						Dedekind::BIGINTTAG);
			}

			double final = MPI::Wtime();
			cerr << "Rank " << rank << " exiting! Total: "
					<< final - start << "s\n";
		}
	}
	else
	{
		cout << "Usage: mpirun -np N ./project -d X \n"
			<< "Where X in [2..n) is the Dedekind Number to calculate.\n"
			<< "And N is the number of processes you would like to use.\n\n"
			<< "Note: The program will also work when running normally "
			<< "(without using mpirun).\n"
			<< "In that case the program will just run on 1 core.\n";
	}
	MPI::Finalize();
}


