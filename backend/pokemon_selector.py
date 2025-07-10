import requests
import random
import json

class PokemonSelector:
    def __init__(self):
        self.base_url = "https://pokeapi.co/api/v2/"
        self.stats = ["hp", "attack", "defense", "special-attack", "special-defense", "speed"]
        
        # Configure session with better SSL handling
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'Pokemon-Selector/1.0',
            'Accept': 'application/json'
        })
    
    def get_pokemon_data(self, pokemon_name):
        """Fetch Pokemon data from PokeAPI"""
        try:
            url = f"{self.base_url}pokemon/{pokemon_name.lower()}"
            print(f"Connecting to: {url}")
            
            # Try with SSL verification first
            try:
                response = self.session.get(url, timeout=15, verify=True)
                response.raise_for_status()
                return response.json()
            except requests.exceptions.SSLError:
                print("SSL verification failed, trying without SSL verification...")
                # Retry without SSL verification
                response = self.session.get(url, timeout=15, verify=False)
                response.raise_for_status()
                return response.json()
                
        except requests.exceptions.HTTPError as e:
            if e.response.status_code == 404:
                print(f"Pokemon '{pokemon_name}' not found. Check the spelling and try again.")
            else:
                print(f"HTTP error {e.response.status_code}: {e}")
            return None
        except requests.exceptions.Timeout:
            print("Connection timeout. Please check your internet connection and try again.")
            return None
        except requests.exceptions.ConnectionError:
            print("Connection error. Please check your internet connection and try again.")
            return None
        except requests.exceptions.RequestException as e:
            print(f"Network error: {e}")
            return None
        except Exception as e:
            print(f"Unexpected error: {e}")
            return None
    
    def get_pokemon_moves(self, pokemon_data):
        """Extract available moves from Pokemon data"""
        moves = []
        for move in pokemon_data['moves']:
            moves.append(move['move']['name'])
        return moves
    
    def generate_random_ivs(self):
        """Generate random IVs (0-31 for each stat)"""
        return {stat: random.randint(0, 31) for stat in self.stats}
    
    def generate_random_evs(self):
        """Generate random EVs (total max 510, max 252 per stat)"""
        evs = {stat: 0 for stat in self.stats}
        remaining_evs = 510
        
        # Distribute EVs randomly while respecting constraints
        for stat in self.stats[:-1]:  # Leave last stat for remainder
            if remaining_evs > 0:
                max_for_stat = min(252, remaining_evs)
                ev_value = random.randint(0, max_for_stat)
                evs[stat] = ev_value
                remaining_evs -= ev_value
        
        # Put remaining EVs in the last stat (if any)
        evs[self.stats[-1]] = min(252, remaining_evs)
        
        return evs
    
    def get_user_ivs(self):
        """Get IVs from user input"""
        print("\nEnter IVs (0-31) for each stat (or press Enter for random):")
        ivs = {}
        
        for stat in self.stats:
            while True:
                user_input = input(f"{stat.capitalize()} IV (0-31): ").strip()
                
                if user_input == "":
                    ivs[stat] = random.randint(0, 31)
                    print(f"Random {stat} IV: {ivs[stat]}")
                    break
                
                try:
                    iv_value = int(user_input)
                    if 0 <= iv_value <= 31:
                        ivs[stat] = iv_value
                        break
                    else:
                        print("IV must be between 0 and 31.")
                except ValueError:
                    print("Please enter a valid number.")
        
        return ivs
    
    def get_user_evs(self):
        """Get EVs from user input"""
        print("\nEnter EVs (0-252, total max 510) for each stat (or press Enter for random):")
        evs = {}
        total_evs = 0
        
        for stat in self.stats:
            while True:
                remaining = 510 - total_evs
                if remaining <= 0:
                    evs[stat] = 0
                    print(f"{stat.capitalize()} EV: 0 (no EVs remaining)")
                    break
                
                user_input = input(f"{stat.capitalize()} EV (0-{min(252, remaining)}): ").strip()
                
                if user_input == "":
                    max_ev = min(252, remaining)
                    evs[stat] = random.randint(0, max_ev)
                    print(f"Random {stat} EV: {evs[stat]}")
                    total_evs += evs[stat]
                    break
                
                try:
                    ev_value = int(user_input)
                    if 0 <= ev_value <= min(252, remaining):
                        evs[stat] = ev_value
                        total_evs += ev_value
                        break
                    else:
                        print(f"EV must be between 0 and {min(252, remaining)}.")
                except ValueError:
                    print("Please enter a valid number.")
        
        return evs
    
    def get_user_moves(self, available_moves):
        """Get moves from user input"""
        print(f"\nAvailable moves ({len(available_moves)} total):")
        print("First 20 moves:", ", ".join(available_moves[:20]))
        if len(available_moves) > 20:
            print("... and more")
        
        print("\nSelect 4 moves (or type 'random' for random selection):")
        moves = []
        
        for i in range(4):
            while True:
                user_input = input(f"Move {i+1}: ").strip().lower()
                
                if user_input == "random":
                    remaining_moves = [move for move in available_moves if move not in moves]
                    if remaining_moves:
                        selected_move = random.choice(remaining_moves)
                        moves.append(selected_move)
                        print(f"Random move selected: {selected_move}")
                        break
                    else:
                        print("No more moves available for random selection.")
                elif user_input in [move.lower() for move in available_moves]:
                    # Find the actual move name with correct capitalization
                    actual_move = next(move for move in available_moves if move.lower() == user_input)
                    if actual_move not in moves:
                        moves.append(actual_move)
                        break
                    else:
                        print("Move already selected. Choose a different move.")
                elif user_input == "":
                    print("Please enter a move name or 'random'.")
                else:
                    print("Move not available. Please choose from the available moves or type 'random'.")
        
        return moves
    
    def display_pokemon_summary(self, pokemon_data, ivs, evs, moves):
        """Display the final Pokemon summary"""
        print("\n" + "="*50)
        print("POKEMON SUMMARY")
        print("="*50)
        print(f"Name: {pokemon_data['name'].capitalize()}")
        print(f"Height: {pokemon_data['height']/10} m")
        print(f"Weight: {pokemon_data['weight']/10} kg")
        print(f"Types: {', '.join([t['type']['name'].capitalize() for t in pokemon_data['types']])}")
        
        print("\nBase Stats:")
        for stat in pokemon_data['stats']:
            print(f"  {stat['stat']['name'].capitalize()}: {stat['base_stat']}")
        
        print("\nIVs:")
        for stat, iv in ivs.items():
            print(f"  {stat.capitalize()}: {iv}")
        
        print("\nEVs:")
        for stat, ev in evs.items():
            print(f"  {stat.capitalize()}: {ev}")
        
        print("\nSelected Moves:")
        for i, move in enumerate(moves, 1):
            print(f"  {i}. {move.replace('-', ' ').title()}")
        
        print("="*50)
    
    def run(self):
        """Main application loop"""
        print("Welcome to the Pokemon Selector!")
        print("This app fetches Pokemon data from PokeAPI.")
        print("Type 'quit' to exit the program.")
        
        while True:
            print("\n" + "-"*40)
            pokemon_name = input("Enter a Pokemon name: ").strip()
            
            if pokemon_name.lower() == 'quit':
                print("Thanks for using Pokemon Selector!")
                break
            
            if not pokemon_name:
                print("Please enter a Pokemon name.")
                continue
            
            # Fetch Pokemon data
            print(f"Fetching data for {pokemon_name}...")
            pokemon_data = self.get_pokemon_data(pokemon_name)
            
            if not pokemon_data:
                print("Please try again with a different Pokemon name.")
                continue
            
            print(f"Found {pokemon_data['name'].capitalize()}!")
            
            # Get available moves
            available_moves = self.get_pokemon_moves(pokemon_data)
            
            # Get IVs
            print("\n--- IV Selection ---")
            choice = input("Enter IVs manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                ivs = self.generate_random_ivs()
                print("Random IVs generated!")
            else:
                ivs = self.get_user_ivs()
            
            # Get EVs
            print("\n--- EV Selection ---")
            choice = input("Enter EVs manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                evs = self.generate_random_evs()
                print("Random EVs generated!")
            else:
                evs = self.get_user_evs()
            
            # Get Moves
            print("\n--- Move Selection ---")
            choice = input("Select moves manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                moves = random.sample(available_moves, min(4, len(available_moves)))
                print("Random moves selected!")
            else:
                moves = self.get_user_moves(available_moves)
            
            # Display summary
            self.display_pokemon_summary(pokemon_data, ivs, evs, moves)
            
            # Ask if user wants to continue
            continue_choice = input("\nWould you like to select another Pokemon? (y/n): ").strip().lower()
            if continue_choice != 'y':
                print("Thanks for using Pokemon Selector!")
                break

if __name__ == "__main__":
    selector = PokemonSelector()
    selector.run()
