make clean 2>&1 1>/dev/null;make 2>&1 1>/dev/null;

echo "=======MSI: traces/canneal.04t.debug======"
./smp_cache 8192 8 64 4 0 traces/canneal.04t.debug >output.txt;
diff -iyw --suppress-common output.txt traces/MSI_c_4.out

echo "=======MSI: traces/canneal.04t.longTrace======"
./smp_cache 8192 8 64 4 0 traces/canneal.04t.longTrace >output.txt;
diff -iyw --suppress-common output.txt traces/MSI_c_4.long.out

echo "=======MESI: traces/canneal.04t.debug======"
./smp_cache 8192 8 64 4 1 traces/canneal.04t.debug >output.txt;
diff -iyw --suppress-common output.txt traces/MESI_c_4.out

echo "=======MESI: traces/canneal.04t.longTrace======"
./smp_cache 8192 8 64 4 1 traces/canneal.04t.longTrace >output.txt;
diff -iyw --suppress-common output.txt traces/MESI_c_4.long.out

echo "=======Dragon: traces/canneal.04t.debug======"
./smp_cache 8192 8 64 4 2 traces/canneal.04t.debug >output.txt;
diff -iyw --suppress-common output.txt traces/Dragon_c_4.out

echo "=======Dragon: traces/canneal.04t.longTrace======"
./smp_cache 8192 8 64 4 2 traces/canneal.04t.longTrace >output.txt;
diff -iyw --suppress-common output.txt traces/Dragon_c_4.long.out

